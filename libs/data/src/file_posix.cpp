// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_data/file.hpp>

#include <seir_base/scope.hpp>
#include <seir_data/blob.hpp>

#include <fcntl.h>    // open
#include <stdio.h>    // perror
#include <sys/mman.h> // mmap, munmap
#include <unistd.h>   // close, lseek

namespace
{
	class Descriptor
	{
	public:
		constexpr explicit Descriptor(int descriptor) noexcept
			: _descriptor{ descriptor } {}
		~Descriptor() noexcept
		{
			if (_descriptor != -1 && ::close(_descriptor) == -1)
				::perror("close");
		}
		[[nodiscard]] constexpr operator int() const noexcept { return _descriptor; }

	private:
		int _descriptor = -1;
	};

	struct FileBlob final : seir::Blob
	{
		FileBlob(void*& data, size_t size) noexcept
			: Blob{ data, size }
		{
			data = MAP_FAILED;
		}

		~FileBlob() noexcept override
		{
			if (_data != MAP_FAILED && ::munmap(const_cast<void*>(_data), _size) == -1)
				::perror("munmap");
		}
	};
}

namespace seir
{
	UniquePtr<Blob> openFile(const std::filesystem::path& path)
	{
		if (const Descriptor file{ ::open(path.c_str(), O_RDONLY | O_NOATIME) }; file == -1)
			::perror("open");
		else if (const auto size = ::lseek(file, 0, SEEK_END); size == -1)
			::perror("lseek");
		else if (auto data = ::mmap(nullptr, static_cast<size_t>(size), PROT_READ, MAP_PRIVATE, file, 0); data == MAP_FAILED)
			::perror("mmap");
		else
		{
			SEIR_FINALLY([&] {
				if (data != MAP_FAILED && ::munmap(data, static_cast<size_t>(size)) == -1)
					::perror("munmap");
			});
			return makeUnique<Blob, FileBlob>(data, static_cast<size_t>(size));
		}
		return {};
	}
}
