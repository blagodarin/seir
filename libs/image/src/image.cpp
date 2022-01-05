// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_image/image.hpp>

#include <seir_base/endian.hpp>
#include <seir_data/reader.hpp>
#include "format.hpp"

namespace seir
{
	std::optional<Image> Image::load(const SharedPtr<Blob>& blob)
	{
		if (blob && blob->size() >= 4)
		{
			Image result;
			switch (Reader reader{ *blob }; *static_cast<const uint16_t*>(blob->data()))
			{
			case makeCC('B', 'M'):
#if SEIR_IMAGE_BMP
				result._data = loadBmpImage(reader, result._info);
#endif
				break;
			case makeCC('D', 'D'):
				// TODO: Load DDS image.
				break;
			case makeCC('\xff', '\xd8'): // SOI marker.
				// TODO: Load JPEG image.
				break;
			case makeCC('\x89', 'P'):
				// TODO: Load PNG image.
				break;
			default:
				// ICO files start with [00 00] (reserved, must be zero) followed by [01 00] (file type, 1 is ICO).
				// Supported TGA files start with [xx 00 02 00] or [xx 00 03 00] (xx is usually zero).
				if (*static_cast<const uint32_t*>(blob->data()) == makeCC('\x00', '\x00', '\x01', '\x00'))
				{
#if SEIR_IMAGE_ICO
					result._data = loadIcoImage(reader, result._info);
#endif
				}
				else
				{
#if SEIR_IMAGE_TGA
					result._data = loadTgaImage(reader, result._info);
#endif
				}
			}
			if (result._data)
				return result;
		}
		return {};
	}

	Image::Image() noexcept = default;
	Image::Image(Image&&) noexcept = default;
	Image& Image::operator=(Image&&) noexcept = default;
	Image::~Image() noexcept = default;

	Image::Image(const ImageInfo& info, Buffer<std::byte>&& buffer) noexcept
		: _info{ info }, _data{ buffer.data() }, _buffer{ std::move(buffer) } {}
}
