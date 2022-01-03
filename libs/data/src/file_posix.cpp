// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_base/scope.hpp>
#include <seir_data/blob.hpp>
#include <seir_data/save_file.hpp>
#include <seir_data/temporary_file.hpp>

#include <cstdio>     // perror, rename
#include <fcntl.h>    // open
#include <sys/mman.h> // mmap, munmap
#include <unistd.h>   // close, fsync, lseek, pwrite, unlink

namespace
{
	struct Descriptor
	{
		int _descriptor = -1;
		constexpr explicit Descriptor(int descriptor) noexcept
			: _descriptor{ descriptor } {}
		constexpr Descriptor(Descriptor&& other) noexcept
			: _descriptor{ other._descriptor } { other._descriptor = -1; }
		~Descriptor() noexcept
		{
			if (_descriptor != -1 && ::close(_descriptor) == -1)
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
		static seir::SharedPtr<seir::Blob> create(int descriptor, size_t size)
		{
			if (auto data = ::mmap(nullptr, size, PROT_READ, MAP_PRIVATE, descriptor, 0); data == kMapFailed)
				::perror("mmap");
			else
			{
				SEIR_FINALLY([&] {
					if (data != kMapFailed && ::munmap(data, size) == -1)
						::perror("munmap");
				});
				return seir::makeShared<seir::Blob, FileBlob>(data, size);
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
		constexpr explicit FileWriter(Descriptor&& file) noexcept
			: _file{ std::move(file) } {}
		bool flush() noexcept override { return ::flushFile(_file._descriptor); }
		bool reserveImpl(uint64_t) noexcept override { return true; }
		bool writeImpl(uint64_t offset, const void* data, size_t size) noexcept override { return ::writeFile(_file._descriptor, offset, data, size); }
	};

	struct SaveFileImpl final : seir::SaveFile
	{
		const Descriptor _file;
		const std::string _path;
		const std::string _temporaryPath;
		bool _committed = false;
		SaveFileImpl(Descriptor&& file, std::string&& path, std::string&& temporaryPath) noexcept
			: _file{ std::move(file) }, _path{ std::move(path) }, _temporaryPath{ std::move(temporaryPath) } {}
		~SaveFileImpl() noexcept override
		{
			if (!_committed && ::unlink(_temporaryPath.c_str()))
				::perror("unlink");
		}
		bool flush() noexcept override { return ::flushFile(_file._descriptor); }
		bool reserveImpl(uint64_t) noexcept override { return true; }
		bool writeImpl(uint64_t offset, const void* data, size_t size) noexcept override { return ::writeFile(_file._descriptor, offset, data, size); }
	};

	struct TemporaryWriterImpl final : seir::TemporaryWriter
	{
		Descriptor _file;
		std::string _path;
		TemporaryWriterImpl(Descriptor&& file, std::string&& path) noexcept
			: _file{ std::move(file) }, _path{ std::move(path) } {}
		~TemporaryWriterImpl() noexcept override
		{
			if (_file._descriptor != -1 && ::unlink(_path.c_str()))
				::perror("unlink");
		}
		bool flush() noexcept override { return ::flushFile(_file._descriptor); }
		bool reserveImpl(uint64_t) noexcept override { return true; }
		bool writeImpl(uint64_t offset, const void* data, size_t size) noexcept override { return ::writeFile(_file._descriptor, offset, data, size); }
	};

	struct TemporaryFileImpl final : seir::TemporaryFile
	{
		const Descriptor _file;
		const uint64_t _size;
		TemporaryFileImpl(std::string&& path, Descriptor&& file, uint64_t size) noexcept
			: TemporaryFile{ std::move(path) }, _file{ std::move(file) }, _size{ size } {}
		~TemporaryFileImpl() noexcept override
		{
			if (::unlink(_path.c_str()))
				::perror("unlink");
		}
	};
}

namespace seir
{
	SharedPtr<Blob> Blob::from(const std::string& path)
	{
		constexpr int flags = O_RDONLY | O_CLOEXEC
#ifdef __linux__
			| O_NOATIME
#endif
			;
		if (const Descriptor file{ ::open(path.c_str(), flags) }; file._descriptor == -1)
			::perror("open");
		else if (const auto size = ::lseek(file._descriptor, 0, SEEK_END); size == -1)
			::perror("lseek");
		else
			return FileBlob::create(file._descriptor, static_cast<uint64_t>(size));
		return {};
	}

	SharedPtr<Blob> Blob::from(TemporaryFile& file)
	{
		const auto& impl = static_cast<const TemporaryFileImpl&>(file);
		return FileBlob::create(impl._file._descriptor, impl._size);
	}

	UniquePtr<SaveFile> SaveFile::create(std::string&& path)
	{
		const auto separator = path.rfind('/');
		if (separator == path.size() - 1)
			return {};
		auto temporaryPath = path + ".XXXXXX";
		if (Descriptor file{ ::mkstemp(temporaryPath.data()) }; file._descriptor == -1)
			::perror("mkstemp");
		else // TODO: Copy access mode from the original file.
			return makeUnique<SaveFile, SaveFileImpl>(std::move(file), std::move(path), std::move(temporaryPath));
		return {};
	}

	bool SaveFile::commit(UniquePtr<SaveFile>&& file) noexcept
	{
		if (const auto impl = staticCast<SaveFileImpl>(std::move(file)))
		{
			if (::fsync(impl->_file._descriptor))
				::perror("fsync");
			else if (::rename(impl->_temporaryPath.c_str(), impl->_path.c_str()))
				::perror("rename");
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
		// TODO: Use TMPDIR environment variable.
		// TODO: Use O_TMPFILE on Linux.
		std::string path = "/tmp/seir.XXXXXX";
		if (Descriptor file{ ::mkstemp(path.data()) }; file._descriptor == -1)
			::perror("mkstemp");
		else
			return makeUnique<TemporaryWriter, TemporaryWriterImpl>(std::move(file), std::move(path));
		return {};
	}

	UniquePtr<TemporaryFile> TemporaryWriter::commit(UniquePtr<TemporaryWriter>&& writer)
	{
		if (const auto impl = staticCast<TemporaryWriterImpl>(std::move(writer)))
			return makeUnique<TemporaryFile, TemporaryFileImpl>(std::move(impl->_path), std::move(impl->_file), impl->size());
		return {};
	}

	UniquePtr<Writer> Writer::create(const std::string& path)
	{
		constexpr int flags = O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC
#ifdef __linux__
			| O_NOATIME
#endif
			;
		if (Descriptor file{ ::open(path.c_str(), flags, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) }; file._descriptor != -1)
			return makeUnique<Writer, FileWriter>(std::move(file));
		::perror("open");
		return {};
	}
}
