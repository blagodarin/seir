// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_data/file.hpp>

#include <seir_base/scope.hpp>
#include <seir_data/blob.hpp>
#include <seir_data/writer.hpp>

#include <fcntl.h>    // open
#include <stdio.h>    // perror
#include <sys/mman.h> // mmap, munmap
#include <unistd.h>   // close, ftruncate, lseek, pwrite, unlink

namespace
{
	struct Descriptor
	{
		int _descriptor = -1;
		const bool _close;
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

	struct FileBlob final : seir::Blob
	{
		FileBlob(void*& data, size_t size) noexcept
			: Blob{ data, size } { data = MAP_FAILED; }
		~FileBlob() noexcept override
		{
			if (_data != MAP_FAILED && ::munmap(const_cast<void*>(_data), _size) == -1)
				::perror("munmap");
		}
		static seir::UniquePtr<seir::Blob> create(int descriptor)
		{
			if (const auto size = ::lseek(descriptor, 0, SEEK_END); size == -1)
				::perror("lseek");
			else if (auto data = ::mmap(nullptr, static_cast<size_t>(size), PROT_READ, MAP_PRIVATE, descriptor, 0); data == MAP_FAILED)
				::perror("mmap");
			else
			{
				SEIR_FINALLY([&] {
					if (data != MAP_FAILED && ::munmap(data, static_cast<size_t>(size)) == -1)
						::perror("munmap");
				});
				return seir::makeUnique<seir::Blob, FileBlob>(data, static_cast<size_t>(size));
			}
			return {};
		}
	};

	struct FileWriter final : seir::Writer
	{
		const Descriptor _file;
		constexpr explicit FileWriter(int descriptor) noexcept
			: _file{ descriptor, false } {}
		constexpr explicit FileWriter(Descriptor&& file) noexcept
			: _file{ std::move(file) } {}
		bool reserveImpl(uint64_t) noexcept override { return true; }
		bool writeImpl(uint64_t offset, const void* data, size_t size) noexcept override
		{
			const auto result = ::pwrite(_file._descriptor, data, size, static_cast<int64_t>(offset));
			return result != -1;
		}
	};

	struct TemporaryFileImpl final : seir::TemporaryFile
	{
		const Descriptor _file;
		TemporaryFileImpl(std::filesystem::path&& path, Descriptor&& file) noexcept
			: TemporaryFile(std::move(path)), _file{ std::move(file) } {}
		~TemporaryFileImpl() noexcept override
		{
			if (::unlink(_path.c_str()))
				::perror("unlink");
		}
	};
}

namespace seir
{
	UniquePtr<TemporaryFile> TemporaryFile::create()
	{
		auto path = (std::filesystem::temp_directory_path() / "SeirXXXXXX").string();
		if (Descriptor file{ ::mkstemp(path.data()), true }; file._descriptor != -1)
			return makeUnique<TemporaryFile, TemporaryFileImpl>(std::move(path), std::move(file));
		::perror("mkstemp");
		return {};
	}

	UniquePtr<Blob> createFileBlob(const std::filesystem::path& path)
	{
		if (const Descriptor file{ ::open(path.c_str(), O_RDONLY | O_NOATIME), true }; file._descriptor != -1)
			return FileBlob::create(file._descriptor);
		::perror("open");
		return {};
	}

	UniquePtr<Blob> createFileBlob(TemporaryFile& file)
	{
		return FileBlob::create(static_cast<const TemporaryFileImpl&>(file)._file._descriptor);
	}

	UniquePtr<Writer> createFileWriter(const std::filesystem::path& path)
	{
		if (Descriptor file{ ::open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH), true }; file._descriptor != -1)
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
