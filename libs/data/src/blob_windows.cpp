// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_data/blob.hpp>

#include <seir_base/int_utils.hpp>
#include <seir_base/pointer.hpp>

#include <limits>

#define NOGDI
#define NOIME
#define NOKERNEL
#define NOMCX
#define NOMINMAX
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

	struct FileBlob final : seir::Blob
	{
		FileBlob(const void* data, size_t size) noexcept
			: Blob{ data, size } {}
		~FileBlob() noexcept override { ::UnmapViewOfFile(_data); }
	};
}

namespace seir
{
	std::shared_ptr<Blob> Blob::from(const std::filesystem::path& path)
	{
		if (HandlePtr file{ ::CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr) }; file != INVALID_HANDLE_VALUE)
			if (LARGE_INTEGER size; ::GetFileSizeEx(file, &size))
			{
				if (!size.QuadPart)
					return Blob::from(nullptr, 0);
				if (toUnsigned(size.QuadPart) <= std::numeric_limits<size_t>::max())
					if (HandlePtr mapping{ ::CreateFileMappingW(file, nullptr, PAGE_READONLY, toUnsigned(size.HighPart), size.LowPart, nullptr) })
						if (const auto data = ::MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, static_cast<SIZE_T>(size.QuadPart)))
							return std::make_shared<FileBlob>(data, static_cast<size_t>(size.QuadPart));
			}
		return {};
	}
}
