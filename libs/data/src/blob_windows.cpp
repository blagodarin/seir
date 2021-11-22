// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_data/blob.hpp>

#include <seir_base/int_utils.hpp>
#include <seir_base/scope.hpp>

#include <limits>

#define NONLS
#define NOUSER
#define WIN32_LEAN_AND_MEAN
#include <seir_base/windows_utils.hpp>

namespace
{
	struct FileBlob final : seir::Blob
	{
		FileBlob(void*& data, size_t size) noexcept
			: Blob{ data, size }
		{
			data = nullptr;
		}

		~FileBlob() noexcept override
		{
			if (!::UnmapViewOfFile(_data))
				seir::windows::reportLastError("UnmapViewOfFile");
		}
	};
}

namespace seir
{
	UniquePtr<Blob> Blob::from(const std::filesystem::path& path)
	{
		if (windows::Handle file{ ::CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr) }; file == INVALID_HANDLE_VALUE)
			windows::reportLastError("CreateFileW");
		else if (LARGE_INTEGER size; !::GetFileSizeEx(file, &size))
			windows::reportLastError("GetFileSizeEx");
		else
		{
			if (!size.QuadPart)
				return Blob::from(nullptr, 0);
			if (toUnsigned(size.QuadPart) <= std::numeric_limits<size_t>::max())
				if (windows::Handle mapping{ ::CreateFileMappingW(file, nullptr, PAGE_READONLY, toUnsigned(size.HighPart), size.LowPart, nullptr) }; !mapping)
					windows::reportLastError("CreateFileMappingW");
				else if (auto data = ::MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, static_cast<SIZE_T>(size.QuadPart)); !data)
					windows::reportLastError("MapViewOfFile");
				else
				{
					SEIR_FINALLY([&data] {
						if (data && !::UnmapViewOfFile(data))
							windows::reportLastError("UnmapViewOfFile");
					});
					return makeUnique<Blob, FileBlob>(data, static_cast<size_t>(size.QuadPart));
				}
		}
		return {};
	}
}
