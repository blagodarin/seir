// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_data/file.hpp>

#include <seir_base/int_utils.hpp>
#include <seir_base/scope.hpp>
#include <seir_data/blob.hpp>
#include <seir_data/writer.hpp>

#define NONLS
#define NOUSER
#define WIN32_LEAN_AND_MEAN
#include <seir_base/windows_utils.hpp>

namespace
{
	struct FileBlob final : seir::Blob
	{
		FileBlob(void*& data, size_t size) noexcept
			: Blob{ data, size } { data = nullptr; }
		~FileBlob() noexcept override
		{
			if (!::UnmapViewOfFile(_data))
				seir::windows::reportError("UnmapViewOfFile");
		}
	};

	struct FileWriter final : seir::Writer
	{
		const seir::windows::Handle _handle;
		explicit FileWriter(seir::windows::Handle&& handle) noexcept
			: _handle{ std::move(handle) } {}
		bool reserveImpl(uint64_t) noexcept override { return true; }
		bool writeImpl(uint64_t offset, const void* data, size_t size) noexcept override
		{
			DWORD bytesWritten = 0;
			OVERLAPPED overlapped{};
			overlapped.Offset = static_cast<DWORD>(offset);
			overlapped.OffsetHigh = static_cast<DWORD>(offset >> 32);
			if (::WriteFile(_handle, data, static_cast<DWORD>(size), &bytesWritten, &overlapped))
				return bytesWritten == size;
			seir::windows::reportError("UnmapViewOfFile");
			return false;
		}
	};

	struct TemporaryFileImpl final : seir::TemporaryFile
	{
		explicit TemporaryFileImpl(std::filesystem::path&& path) noexcept
			: TemporaryFile(std::move(path)) {}
		~TemporaryFileImpl() noexcept override
		{
			if (!::DeleteFileW(_path.c_str()))
				seir::windows::reportError("DeleteFileW");
		}
	};
}

namespace seir
{
	UniquePtr<TemporaryFile> TemporaryFile::create()
	{
		constexpr auto maxTempPathSize = MAX_PATH - 14; // GetTempFileName path length limit.
		std::array<wchar_t, maxTempPathSize + 1> path;
		static_assert(path.size() <= std::numeric_limits<DWORD>::max());
		if (!::GetTempPathW(static_cast<DWORD>(path.size()), path.data()))
			windows::reportError("GetTempPathW");
		else
		{
			std::array<wchar_t, MAX_PATH> name;
			if (const auto status = ::GetTempFileNameW(path.data(), L"Seir", 0, name.data()); !status)
				windows::reportError("GetTempPathW");
			else if (status == ERROR_BUFFER_OVERFLOW)
				windows::reportError("GetTempPathW", status);
			else
				return makeUnique<TemporaryFile, TemporaryFileImpl>(name.data());
		}
		return {};
	}

	SharedPtr<Blob> createFileBlob(const std::filesystem::path& path)
	{
		if (windows::Handle file{ ::CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr) }; file == INVALID_HANDLE_VALUE)
			windows::reportError("CreateFileW");
		else if (LARGE_INTEGER size; !::GetFileSizeEx(file, &size))
			windows::reportError("GetFileSizeEx");
		else
		{
			if (!size.QuadPart)
				return Blob::from(nullptr, 0);
			if (toUnsigned(size.QuadPart) <= std::numeric_limits<size_t>::max())
				if (windows::Handle mapping{ ::CreateFileMappingW(file, nullptr, PAGE_READONLY, toUnsigned(size.HighPart), size.LowPart, nullptr) }; !mapping)
					windows::reportError("CreateFileMappingW");
				else if (auto data = ::MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, static_cast<SIZE_T>(size.QuadPart)); !data)
					windows::reportError("MapViewOfFile");
				else
				{
					SEIR_FINALLY([&data] {
						if (data && !::UnmapViewOfFile(data))
							windows::reportError("UnmapViewOfFile");
					});
					return makeShared<Blob, FileBlob>(data, static_cast<size_t>(size.QuadPart));
				}
		}
		return {};
	}

	// cppcheck-suppress constParameter
	SharedPtr<Blob> createFileBlob(TemporaryFile& file)
	{
		return createFileBlob(file.path());
	}

	UniquePtr<Writer> createFileWriter(const std::filesystem::path& path)
	{
		if (windows::Handle file{ ::CreateFileW(path.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr) }; file != INVALID_HANDLE_VALUE)
			return makeUnique<Writer, FileWriter>(std::move(file));
		windows::reportError("CreateFileW");
		return {};
	}

	// cppcheck-suppress constParameter
	UniquePtr<Writer> createFileWriter(TemporaryFile& file)
	{
		return createFileWriter(file.path());
	}
}
