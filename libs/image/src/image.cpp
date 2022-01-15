// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_image/image.hpp>

#include <seir_base/endian.hpp>
#include "format.hpp"

#include <algorithm>

// TODO: Add support for:
// - writing image data at aligned offsets (for more efficient copying of memory-mapped data);
// - saving screenshots (with default names and to the default screenshot location);
// - loading image data into the specified buffer (e.g. mapped texture memory);
// - compressed pixel formats (e.g. S3TC);
// - multi-layer images (e.g. textures with mipmaps);
// - separate image header/data loading;
// - some sort of image packs (to be able to pre-load image headers and load image data separately).

namespace seir
{
	std::optional<Image> Image::load(const SharedPtr<Blob>& blob)
	{
		if (blob && blob->size() >= 4)
		{
			Image result;
			switch (Reader reader{ *blob }; *static_cast<const uint16_t*>(blob->data()))
			{
			case kBmpFileID:
#if SEIR_IMAGE_BMP
				result._data = loadBmpImage(reader, result._info);
#endif
				break;
			case makeCC('D', 'D'):
				// TODO: Load DDS image.
				break;
			case kJpegFileID:
#if SEIR_IMAGE_JPEG
				result._data = loadJpegImage(reader, result._info, result._buffer);
#endif
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
			{
				if (!result._buffer.capacity())
					result._blob = blob;
				return result;
			}
		}
		return {};
	}

	Image::Image() noexcept = default;
	Image::Image(Image&&) noexcept = default;
	Image& Image::operator=(Image&&) noexcept = default;
	Image::~Image() noexcept = default;

	Image::Image(const ImageInfo& info, Buffer<>&& buffer) noexcept
		: _info{ info }, _data{ buffer.data() }, _buffer{ std::move(buffer) } {}

	bool Image::save(ImageFormat format, Writer& writer, [[maybe_unused]] int compressionLevel) const noexcept
	{
		switch (format)
		{
		case ImageFormat::Tga:
#if SEIR_IMAGE_TGA
			return saveTgaImage(writer, _info, _data);
#else
			break;
#endif
		case ImageFormat::Jpeg:
#if SEIR_IMAGE_JPEG
			return saveJpegImage(writer, _info, _data, std::clamp(compressionLevel, 0, 100));
#else
			break;
#endif
		}
		return false;
	}
}
