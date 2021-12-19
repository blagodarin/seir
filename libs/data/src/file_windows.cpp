// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_data/file.hpp>

#include <seir_base/scope.hpp>
#include <seir_data/blob.hpp>
#include <seir_data/writer.hpp>

#include <array>

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
		static seir::SharedPtr<Blob> create(const wchar_t* path)
		{
			if (const seir::windows::Handle file{ ::CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr) }; file == INVALID_HANDLE_VALUE)
				seir::windows::reportError("CreateFileW");
			else if (LARGE_INTEGER size{}; !::GetFileSizeEx(file, &size))
				seir::windows::reportError("GetFileSizeEx");
			else
			{
				if (!size.QuadPart)
					return Blob::from(nullptr, 0);
				if (static_cast<ULONGLONG>(size.QuadPart) <= std::numeric_limits<size_t>::max())
					if (const seir::windows::Handle mapping{ ::CreateFileMappingW(file, nullptr, PAGE_READONLY, static_cast<ULONG>(size.HighPart), size.LowPart, nullptr) }; !mapping)
						seir::windows::reportError("CreateFileMappingW");
					else if (auto data = ::MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, static_cast<SIZE_T>(size.QuadPart)); !data)
						seir::windows::reportError("MapViewOfFile");
					else
					{
						SEIR_FINALLY([&data] {
							if (data && !::UnmapViewOfFile(data))
								seir::windows::reportError("UnmapViewOfFile");
						});
						return seir::makeShared<Blob, FileBlob>(data, static_cast<size_t>(size.QuadPart));
					}
			}
			return {};
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
		static seir::UniquePtr<Writer> create(const wchar_t* path)
		{
			if (seir::windows::Handle file{ ::CreateFileW(path, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr) }; file != INVALID_HANDLE_VALUE)
				return seir::makeUnique<Writer, FileWriter>(std::move(file));
			seir::windows::reportError("CreateFileW");
			return {};
		}
	};

	struct TemporaryFileImpl final : seir::TemporaryFile
	{
		const std::wstring _path;
		explicit TemporaryFileImpl(std::wstring&& path) noexcept
			: _path{ std::move(path) } {}
		~TemporaryFileImpl() noexcept override
		{
			if (!::DeleteFileW(_path.c_str()))
				seir::windows::reportError("DeleteFileW");
		}
	};

	struct WPath
	{
		std::array<wchar_t, MAX_PATH + 1> _buffer;
		WPath(std::string_view path) noexcept
		{
			const auto length = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path.data(), static_cast<int>(path.size()), _buffer.data(), static_cast<int>(_buffer.size() - 1));
			if (!length)
				seir::windows::reportError("MultiByteToWideChar");
			_buffer[static_cast<size_t>(length)] = L'\0';
		}
		constexpr explicit operator bool() const noexcept { return _buffer[0] != L'\0'; }
	};
}

namespace seir
{
	UniquePtr<TemporaryFile> TemporaryFile::create()
	{
		constexpr auto maxPathPrefixSize = MAX_PATH - 14; // GetTempFileNameW limit.
		std::array<wchar_t, maxPathPrefixSize + 1> pathPrefix;
		if (!::GetTempPathW(static_cast<DWORD>(pathPrefix.size()), pathPrefix.data()))
			windows::reportError("GetTempPathW");
		else
		{
			std::array<wchar_t, MAX_PATH> path;
			if (const auto status = ::GetTempFileNameW(pathPrefix.data(), L"Seir", 0, path.data()); !status)
				windows::reportError("GetTempFileNameW");
			else if (status == ERROR_BUFFER_OVERFLOW)
				windows::reportError("GetTempFileNameW", status);
			else
				return makeUnique<TemporaryFile, TemporaryFileImpl>(path.data());
		}
		return {};
	}

	SharedPtr<Blob> createFileBlob(const std::string& path)
	{
		if (const WPath wpath{ path })
			return FileBlob::create(wpath._buffer.data());
		return {};
	}

	// cppcheck-suppress constParameter
	SharedPtr<Blob> createFileBlob(TemporaryFile& file)
	{
		return FileBlob::create(static_cast<TemporaryFileImpl&>(file)._path.c_str());
	}

	UniquePtr<Writer> createFileWriter(const std::string& path)
	{
		if (const WPath wpath{ path })
			return FileWriter::create(wpath._buffer.data());
		return {};
	}

	// cppcheck-suppress constParameter
	UniquePtr<Writer> createFileWriter(TemporaryFile& file)
	{
		return FileWriter::create(static_cast<TemporaryFileImpl&>(file)._path.c_str());
	}
}
