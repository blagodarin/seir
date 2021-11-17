// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_data/blob.hpp>

#include <seir_base/pointer.hpp>

#define NOGDI
#define NOIME
#define NOKERNEL
#define NOMCX
#define NONLS
#define NOSERVICE
#define NOUSER
#define WIN32_LEAN_AND_MEAN
#pragma warning(push)
#pragma warning(disable : 5039) // pointer or reference to potentially throwing function passed to 'extern "C"' function under -EHc. Undefined behavior may occur if this function throws an exception.
#include <windows.h>
#pragma warning(pop)

namespace
{
	struct HandleDeleter
	{
		static void free(HANDLE handle) noexcept
		{
			if (handle && handle != INVALID_HANDLE_VALUE)
				::CloseHandle(handle);
		}
	};

	using HandlePtr = seir::Pointer<std::remove_pointer_t<HANDLE>, HandleDeleter>;

	struct FileBlob : seir::Blob
	{
		const HandlePtr _file;
		const HandlePtr _mapping;
		FileBlob(const void* data, size_t size, HandlePtr&& file, HandlePtr&& mapping) noexcept
			: Blob{ data, size }, _file{ std::move(file) }, _mapping{ std::move(mapping) } {}
		~FileBlob() noexcept override { ::UnmapViewOfFile(_data); }
	};
}

namespace seir
{
	std::shared_ptr<Blob> Blob::from(const std::filesystem::path& path)
	{
		if (HandlePtr file{ ::CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr) };
			file != INVALID_HANDLE_VALUE)
			if (LARGE_INTEGER size;
				::GetFileSizeEx(file, &size))
				if (HandlePtr mapping{ ::CreateFileMappingA(file, nullptr, PAGE_READONLY, 0, 0, nullptr) })
					if (const auto data = ::MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0))
						return std::make_shared<FileBlob>(data, static_cast<size_t>(size.QuadPart), std::move(file), std::move(mapping));
		return {};
	}
}
