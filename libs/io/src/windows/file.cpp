// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_base/scope.hpp>
#include <seir_io/blob.hpp>
#include <seir_io/save_file.hpp>
#include <seir_io/temporary.hpp>
#include "utils.hpp"

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

	seir::SharedPtr<seir::Blob> createFileBlob(const wchar_t* path)
	{
		if (const seir::windows::Handle file{ ::CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr) }; file == INVALID_HANDLE_VALUE)
		{
			if (const auto error = ::GetLastError(); error != ERROR_PATH_NOT_FOUND)
				seir::windows::reportError("CreateFileW", error);
		}
		else if (LARGE_INTEGER size{}; !::GetFileSizeEx(file, &size))
			seir::windows::reportError("GetFileSizeEx");
		else
		{
			if (!size.QuadPart)
				return seir::Blob::from(nullptr, 0);
			if (static_cast<ULONGLONG>(size.QuadPart) <= std::numeric_limits<size_t>::max())
				if (const seir::windows::Handle mapping{ ::CreateFileMappingW(file, nullptr, PAGE_READONLY, static_cast<DWORD>(size.HighPart), size.LowPart, nullptr) }; !mapping)
					seir::windows::reportError("CreateFileMappingW");
				else if (auto data = ::MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, static_cast<SIZE_T>(size.QuadPart)); !data)
					seir::windows::reportError("MapViewOfFile");
				else
				{
					SEIR_FINALLY{ [&data]() noexcept {
						if (data && !::UnmapViewOfFile(data))
							seir::windows::reportError("UnmapViewOfFile");
					} };
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

	struct SaveFileImpl final : seir::SaveFile
	{
		seir::windows::Handle _handle;
		const std::wstring _path;
		const std::wstring _temporaryPath;
		bool _committed = false;
		explicit SaveFileImpl(seir::windows::Handle&& handle, std::wstring&& path, std::wstring&& temporaryPath) noexcept
			: _handle{ std::move(handle) }, _path{ std::move(path) }, _temporaryPath{ std::move(temporaryPath) } {}
		~SaveFileImpl() noexcept override
		{
			_handle = {};
			if (!_committed && !::DeleteFileW(_temporaryPath.c_str()))
				seir::windows::reportError("DeleteFileW");
		}
		bool flush() noexcept override { return true; }
		bool reserveImpl(uint64_t) noexcept override { return true; }
		bool writeImpl(uint64_t offset, const void* data, size_t size) noexcept override { return ::writeFile(_handle, offset, data, size); }
	};

	struct TemporaryWriterImpl final : seir::TemporaryWriter
	{
		seir::windows::Handle _handle;
		std::string _path;
		explicit TemporaryWriterImpl(seir::windows::Handle&& handle, std::string&& path) noexcept
			: _handle{ std::move(handle) }, _path{ std::move(path) } {}
		bool flush() noexcept override { return true; }
		bool reserveImpl(uint64_t) noexcept override { return true; }
		bool writeImpl(uint64_t offset, const void* data, size_t size) noexcept override { return ::writeFile(_handle, offset, data, size); }
	};

	struct TemporaryFileImpl final : seir::TemporaryFile
	{
		const seir::windows::Handle _handle;
		const uint64_t _size;
		TemporaryFileImpl(std::string&& path, seir::windows::Handle&& handle, uint64_t size) noexcept
			: TemporaryFile{ std::move(path) }, _handle{ std::move(handle) }, _size{ size } {}
		~TemporaryFileImpl() noexcept override = default;
	};
}

namespace seir
{
	SharedPtr<Blob> Blob::from(const std::string& path)
	{
		if (const windows::WString wpath{ path })
			return ::createFileBlob(wpath.c_str());
		return {};
	}

	SharedPtr<Blob> Blob::from(TemporaryFile& file)
	{
		auto& impl = static_cast<TemporaryFileImpl&>(file);
		if (!impl._size)
			return seir::Blob::from(nullptr, 0);
		if (impl._size <= std::numeric_limits<size_t>::max())
			if (const windows::Handle mapping{ ::CreateFileMappingW(impl._handle, nullptr, PAGE_READONLY, static_cast<DWORD>(impl._size >> 32), static_cast<DWORD>(impl._size), nullptr) }; !mapping)
				windows::reportError("CreateFileMappingW");
			else if (auto data = ::MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, static_cast<SIZE_T>(impl._size)); !data)
				windows::reportError("MapViewOfFile");
			else
			{
				SEIR_FINALLY{ [&data]() noexcept {
					if (data && !::UnmapViewOfFile(data))
						windows::reportError("UnmapViewOfFile");
				} };
				return seir::makeShared<seir::Blob, FileBlob>(data, static_cast<size_t>(impl._size));
			}
		return {};
	}

	UniquePtr<SaveFile> SaveFile::create(std::string&& path)
	{
		if (const windows::WString wide{ path })
		{
			std::wstring wpath{ wide.c_str(), wide.size() };
			std::wstring directory;
			if (const auto separator = wpath.find_last_of(L"/\\"); separator == std::string::npos)
				directory = L".";
			else if (separator < wpath.size() - 1)
				directory = wpath.substr(0, separator);
			else
				return {};
			std::array<wchar_t, MAX_PATH> temporaryPath;
			if (const auto status = ::GetTempFileNameW(directory.data(), L"Sei", 0, temporaryPath.data()); !status)
				windows::reportError("GetTempFileNameW");
			else if (status == ERROR_BUFFER_OVERFLOW)
				windows::reportError("GetTempFileNameW", status);
			else if (windows::Handle file{ ::CreateFileW(temporaryPath.data(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr) }; file == INVALID_HANDLE_VALUE)
				windows::reportError("CreateFileW");
			else
				return makeUnique<SaveFile, SaveFileImpl>(std::move(file), std::move(wpath), temporaryPath.data());
		}
		return {};
	}

	bool SaveFile::commit(UniquePtr<SaveFile>&& file) noexcept
	{
		if (const auto impl = staticCast<SaveFileImpl>(std::move(file)))
			if (::flushFile(impl->_handle))
			{
				impl->_handle = {};
				if (!::ReplaceFileW(impl->_path.c_str(), impl->_temporaryPath.c_str(), nullptr, 0, nullptr, nullptr))
					windows::reportError("ReplaceFileW");
				else
				{
					impl->_committed = true;
					return true;
				}
			}
		return false;
	}

	UniquePtr<TemporaryWriter> TemporaryWriter::create()
	{
		constexpr auto maxPathPrefixSize = MAX_PATH - 14; // GetTempFileNameW limit.
		std::array<wchar_t, maxPathPrefixSize + 1> pathPrefix;
		if (!::GetTempPathW(static_cast<DWORD>(pathPrefix.size()), pathPrefix.data()))
			windows::reportError("GetTempPathW");
		else
		{
			std::array<wchar_t, MAX_PATH> wpath;
			if (const auto status = ::GetTempFileNameW(pathPrefix.data(), L"Sei", 0, wpath.data()); !status)
				windows::reportError("GetTempFileNameW");
			else if (status == ERROR_BUFFER_OVERFLOW)
				windows::reportError("GetTempFileNameW", status);
			else if (windows::Handle file{ ::CreateFileW(wpath.data(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, nullptr) }; file == INVALID_HANDLE_VALUE)
				windows::reportError("CreateFileW");
			else if (const windows::U8String path{ wpath.data() })
				return makeUnique<TemporaryWriter, TemporaryWriterImpl>(std::move(file), path.toString());
		}
		return {};
	}

	UniquePtr<TemporaryFile> TemporaryWriter::commit(UniquePtr<TemporaryWriter>&& writer)
	{
		if (const auto impl = staticCast<TemporaryWriterImpl>(std::move(writer)))
			if (::flushFile(impl->_handle))
				return makeUnique<TemporaryFile, TemporaryFileImpl>(std::move(impl->_path), std::move(impl->_handle), impl->size());
		return {};
	}

	UniquePtr<Writer> Writer::create(const std::filesystem::path& path)
	{
		return ::createFileWriter(path.c_str(), FILE_ATTRIBUTE_NORMAL);
	}

	UniquePtr<Writer> Writer::create(const std::string& path)
	{
		if (const windows::WString wpath{ path })
			return ::createFileWriter(wpath.c_str(), FILE_ATTRIBUTE_NORMAL);
		return {};
	}
}
