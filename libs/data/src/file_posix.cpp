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

#include <cstdio>     // perror, rename
#include <fcntl.h>    // open
#include <sys/mman.h> // mmap, munmap
#include <unistd.h>   // close, fsync, ftruncate, lseek, pwrite, unlink

namespace
{
	struct Descriptor
	{
		int _descriptor = -1;
		bool _close = true;
		constexpr Descriptor(int descriptor, bool close) noexcept
			: _descriptor{ descriptor }, _close{ close } {}
		constexpr Descriptor(Descriptor&& other) noexcept
			: _descriptor{ other._descriptor }, _close{ other._close } { other._descriptor = -1; }
		~Descriptor() noexcept
		{
			if (_close && _descriptor != -1 && ::close(_descriptor) == -1)
				::perror("close");
		}
	};

	const auto kMapFailed = MAP_FAILED; // NOLINT(cppcoreguidelines-pro-type-cstyle-cast, performance-no-int-to-ptr)

	struct FileBlob final : seir::Blob
	{
		FileBlob(void*& data, size_t size) noexcept
			: Blob{ data, size } { data = kMapFailed; }
		~FileBlob() noexcept override
		{
			if (_data != kMapFailed && ::munmap(const_cast<void*>(_data), _size) == -1)
				::perror("munmap");
		}
		static seir::SharedPtr<seir::Blob> create(int descriptor)
		{
			if (const auto size = ::lseek(descriptor, 0, SEEK_END); size == -1)
				::perror("lseek");
			else if (auto data = ::mmap(nullptr, static_cast<size_t>(size), PROT_READ, MAP_PRIVATE, descriptor, 0); data == kMapFailed)
				::perror("mmap");
			else
			{
				SEIR_FINALLY([&] {
					if (data != kMapFailed && ::munmap(data, static_cast<size_t>(size)) == -1)
						::perror("munmap");
				});
				return seir::makeShared<seir::Blob, FileBlob>(data, static_cast<size_t>(size));
			}
			return {};
		}
	};

	bool flushFile(int descriptor) noexcept
	{
		if (!::fsync(descriptor))
			return true;
		::perror("fsync");
		return false;
	}

	bool writeFile(int descriptor, uint64_t offset, const void* data, size_t size) noexcept
	{
		if (::pwrite(descriptor, data, size, static_cast<int64_t>(offset)) == static_cast<ssize_t>(size))
			return true;
		::perror("pwrite");
		return false;
	}

	struct FileWriter final : seir::Writer
	{
		const Descriptor _file;
		constexpr explicit FileWriter(int descriptor) noexcept
			: _file{ descriptor, false } {}
		constexpr explicit FileWriter(Descriptor&& file) noexcept
			: _file{ std::move(file) } {}
		bool flush() noexcept override { return ::flushFile(_file._descriptor); }
		bool reserveImpl(uint64_t) noexcept override { return true; }
		bool writeImpl(uint64_t offset, const void* data, size_t size) noexcept override { return ::writeFile(_file._descriptor, offset, data, size); }
	};

	struct TemporaryFileImpl final : seir::TemporaryFile
	{
		const Descriptor _file;
		TemporaryFileImpl(Descriptor&& file) noexcept
			: _file{ std::move(file) } {}
	};

	struct SaveFileImpl final : seir::SaveFile
	{
		Descriptor _file;
		const std::string _path;
		const std::string _temporaryPath;
		SaveFileImpl(Descriptor&& file, std::string&& path, std::string&& temporaryPath) noexcept
			: _file{ std::move(file) }, _path{ std::move(path) }, _temporaryPath{ std::move(temporaryPath) } {}
		~SaveFileImpl() noexcept override
		{
			if (_file._descriptor != -1 && ::unlink(_temporaryPath.c_str()))
				::perror("unlink");
		}
		bool commit() override
		{
			if (_file._descriptor != -1)
			{
				if (auto file = std::move(_file); ::fsync(file._descriptor) == -1)
				{
					::perror("fsync");
					return false;
				}
				if (!::rename(_temporaryPath.c_str(), _path.c_str()))
					return true;
				if (::unlink(_temporaryPath.c_str()))
					::perror("unlink");
			}
			return false;
		}
		bool flush() noexcept override { return ::flushFile(_file._descriptor); }
		bool reserveImpl(uint64_t) noexcept override { return true; }
		bool writeImpl(uint64_t offset, const void* data, size_t size) noexcept override { return ::writeFile(_file._descriptor, offset, data, size); }
	};
}

namespace seir
{
	UniquePtr<SaveFile> SaveFile::create(std::string&& path)
	{
		const auto separator = path.rfind('/');
		if (separator == path.size() - 1)
			return {};
		auto temporaryPath = path + ".XXXXXX";
		if (Descriptor file{ ::mkstemp(temporaryPath.data()), true }; file._descriptor == -1)
			::perror("mkstemp");
		else // TODO: Copy access mode from the original file.
			return makeUnique<SaveFile, SaveFileImpl>(std::move(file), std::move(path), std::move(temporaryPath));
		return {};
	}

	UniquePtr<TemporaryFile> TemporaryFile::create()
	{
#ifdef __linux__
		if (Descriptor file{ ::open("/tmp", O_RDWR | O_NOATIME | O_CLOEXEC | O_TMPFILE, S_IRUSR | S_IWUSR), true }; file._descriptor != -1)
			return makeUnique<TemporaryFile, TemporaryFileImpl>(std::move(file));
		::perror("open");
#endif
		auto path = std::to_array("/tmp/seir.XXXXXX");
		if (Descriptor file{ ::mkstemp(path.data()), true }; file._descriptor == -1)
			::perror("mkstemp");
		else if (::unlink(path.data()))
			::perror("unlink");
		else
			return makeUnique<TemporaryFile, TemporaryFileImpl>(std::move(file));
		return {};
	}

	SharedPtr<Blob> createFileBlob(const std::string& path)
	{
		constexpr int flags = O_RDONLY | O_CLOEXEC
#ifdef __linux__
			| O_NOATIME
#endif
			;
		if (const Descriptor file{ ::open(path.c_str(), flags), true }; file._descriptor != -1)
			return FileBlob::create(file._descriptor);
		::perror("open");
		return {};
	}

	SharedPtr<Blob> createFileBlob(TemporaryFile& file)
	{
		return FileBlob::create(static_cast<const TemporaryFileImpl&>(file)._file._descriptor);
	}

	UniquePtr<Writer> createFileWriter(const std::string& path)
	{
		constexpr int flags = O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC
#ifdef __linux__
			| O_NOATIME
#endif
			;
		if (Descriptor file{ ::open(path.c_str(), flags, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH), true }; file._descriptor != -1)
			return makeUnique<Writer, FileWriter>(std::move(file));
		::perror("open");
		return {};
	}

	UniquePtr<Writer> createFileWriter(TemporaryFile& file)
	{
		const auto descriptor = static_cast<const TemporaryFileImpl&>(file)._file._descriptor;
		if (::ftruncate(descriptor, 0) == -1)
		{
			::perror("ftruncate");
			return {};
		}
		return makeUnique<Writer, FileWriter>(descriptor);
	}
}
