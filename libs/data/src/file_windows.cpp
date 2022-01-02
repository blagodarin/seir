// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_base/scope.hpp>
#include <seir_data/blob.hpp>
#include <seir_data/file.hpp>
#include <seir_data/save_file.hpp>
#include <seir_data/temporary_file.hpp>
#include <seir_data/writer.hpp>

#include <array>

#define NOUSER
#define WIN32_LEAN_AND_MEAN
#include <seir_base/windows_utils.hpp>

namespace
{
	seir::SharedPtr<seir::Blob> createFileBlob(const wchar_t* path)
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

		if (const seir::windows::Handle file{ ::CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr) }; file == INVALID_HANDLE_VALUE)
			seir::windows::reportError("CreateFileW");
		else if (LARGE_INTEGER size{}; !::GetFileSizeEx(file, &size))
			seir::windows::reportError("GetFileSizeEx");
		else
		{
			if (!size.QuadPart)
				return seir::Blob::from(nullptr, 0);
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
					return seir::makeShared<seir::Blob, FileBlob>(data, static_cast<size_t>(size.QuadPart));
				}
		}
		return {};
	}

	bool flushFile(HANDLE handle) noexcept
	{
		if (::FlushFileBuffers(handle))
			return true;
		seir::windows::reportError("FlushFileBuffers");
		return false;
	}

	bool writeFile(HANDLE handle, uint64_t offset, const void* data, size_t size) noexcept
	{
		DWORD bytesWritten = 0;
		OVERLAPPED overlapped{};
		overlapped.Offset = static_cast<DWORD>(offset);
		overlapped.OffsetHigh = static_cast<DWORD>(offset >> 32);
		if (::WriteFile(handle, data, static_cast<DWORD>(size), &bytesWritten, &overlapped))
			return bytesWritten == size;
		seir::windows::reportError("WriteFile");
		return false;
	}

	seir::UniquePtr<seir::Writer> createFileWriter(const wchar_t* path, DWORD attributes)
	{
		struct FileWriter final : seir::Writer
		{
			const seir::windows::Handle _handle;
			explicit FileWriter(seir::windows::Handle&& handle) noexcept
				: _handle{ std::move(handle) } {}
			bool flush() noexcept override { return ::flushFile(_handle); }
			bool reserveImpl(uint64_t) noexcept override { return true; }
			bool writeImpl(uint64_t offset, const void* data, size_t size) noexcept override { return ::writeFile(_handle, offset, data, size); }
		};

		if (seir::windows::Handle file{ ::CreateFileW(path, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, attributes, nullptr) }; file != INVALID_HANDLE_VALUE)
			return seir::makeUnique<seir::Writer, FileWriter>(std::move(file));
		seir::windows::reportError("CreateFileW");
		return {};
	}

	static seir::UniquePtr<seir::SaveFile> createSaveFile(std::wstring&& path)
	{
		struct SaveFileImpl final : seir::SaveFile
		{
			seir::windows::Handle _handle;
			const std::wstring _path;
			const std::wstring _temporaryPath;
			explicit SaveFileImpl(seir::windows::Handle&& handle, std::wstring&& path, std::wstring&& temporaryPath) noexcept
				: _handle{ std::move(handle) }, _path{ std::move(path) }, _temporaryPath{ std::move(temporaryPath) } {}
			~SaveFileImpl() noexcept override
			{
				if (_handle)
				{
					_handle = {};
					if (!::DeleteFileW(_temporaryPath.c_str()))
						seir::windows::reportError("DeleteFileW");
				}
			}
			bool commit() override
			{
				if (_handle)
				{
					if (auto handle = std::move(_handle); !::flushFile(handle))
						return false;
					if (::ReplaceFileW(_path.c_str(), _temporaryPath.c_str(), nullptr, 0, nullptr, nullptr))
						return true;
					seir::windows::reportError("ReplaceFileW");
					if (!::DeleteFileW(_temporaryPath.c_str()))
						seir::windows::reportError("DeleteFileW");
				}
				return false;
			}
			bool flush() noexcept override { return ::flushFile(_handle); }
			bool reserveImpl(uint64_t) noexcept override { return true; }
			bool writeImpl(uint64_t offset, const void* data, size_t size) noexcept override { return ::writeFile(_handle, offset, data, size); }
		};

		std::wstring directory;
		if (const auto separator = path.find_last_of(L"/\\"); separator == std::string::npos)
			directory = L".";
		else if (separator <= path.size() - 1)
			directory = path.substr(0, separator);
		else
			return {};
		std::array<wchar_t, MAX_PATH> temporaryPath;
		if (const auto status = ::GetTempFileNameW(directory.data(), L"Seir", 0, temporaryPath.data()); !status)
			seir::windows::reportError("GetTempFileNameW");
		else if (status == ERROR_BUFFER_OVERFLOW)
			seir::windows::reportError("GetTempFileNameW", status);
		else if (seir::windows::Handle file{ ::CreateFileW(temporaryPath.data(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr) }; file == INVALID_HANDLE_VALUE)
			seir::windows::reportError("CreateFileW");
		else
			return seir::makeUnique<seir::SaveFile, SaveFileImpl>(std::move(file), std::move(path), temporaryPath.data());
		return {};
	}

	// TODO: Use FILE_FLAG_DELETE_ON_CLOSE.
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
		size_t _size = 0;
		std::array<wchar_t, MAX_PATH + 1> _buffer;
		WPath(std::string_view path) noexcept
		{
			const auto length = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path.data(), static_cast<int>(path.size()), _buffer.data(), static_cast<int>(_buffer.size() - 1));
			if (!length)
				seir::windows::reportError("MultiByteToWideChar");
			_size = static_cast<size_t>(length);
			_buffer[_size] = L'\0';
		}
		constexpr explicit operator bool() const noexcept { return _size > 0; }
	};
}

namespace seir
{
	UniquePtr<SaveFile> SaveFile::create(std::string&& path)
	{
		if (const WPath wpath{ path })
			return ::createSaveFile({ wpath._buffer.data(), wpath._size });
		return {};
	}

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
			return ::createFileBlob(wpath._buffer.data());
		return {};
	}

	// cppcheck-suppress constParameter
	SharedPtr<Blob> createFileBlob(TemporaryFile& file)
	{
		return ::createFileBlob(static_cast<TemporaryFileImpl&>(file)._path.c_str());
	}

	UniquePtr<Writer> createFileWriter(const std::string& path)
	{
		if (const WPath wpath{ path })
			return ::createFileWriter(wpath._buffer.data(), FILE_ATTRIBUTE_NORMAL);
		return {};
	}

	// cppcheck-suppress constParameter
	UniquePtr<Writer> createFileWriter(TemporaryFile& file)
	{
		return ::createFileWriter(static_cast<TemporaryFileImpl&>(file)._path.c_str(), FILE_ATTRIBUTE_TEMPORARY);
	}
}
