// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "format.hpp"

#include "bmp.hpp"

namespace
{
#pragma pack(push, 1)

	// See BITMAPFILEHEADER.
	struct BmpFileHeader
	{
		uint16_t fileType;
		uint32_t fileSize;
		uint32_t reserved;
		uint32_t dataOffset;
	};

#pragma pack(pop)

	static_assert(sizeof(BmpFileHeader) == 14);
}

namespace seir
{
	const void* loadBmpImage(Reader& reader, ImageInfo& info) noexcept
	{
		const auto fileHeader = reader.read<BmpFileHeader>();
		if (!fileHeader
			|| fileHeader->fileType != kBmpFileID
			|| fileHeader->reserved != 0)
			return nullptr;

		const auto bitmapHeader = reader.read<BmpImageHeader>();
		if (!bitmapHeader
			|| bitmapHeader->headerSize < sizeof *bitmapHeader
			|| bitmapHeader->width <= 0
			|| bitmapHeader->height == 0
			|| bitmapHeader->planes != 1
			|| bitmapHeader->compression != BmpCompression::Rgb)
			return nullptr;

		PixelFormat pixelFormat;
		switch (bitmapHeader->bitsPerPixel)
		{
		case 24: pixelFormat = PixelFormat::Bgr24; break;
		case 32: pixelFormat = PixelFormat::Bgra32; break; // Non-standard, it's actually BGRX with an unused byte.
		default: return nullptr;
		}
		if (!reader.seek(fileHeader->dataOffset))
			return nullptr;

		const auto width = static_cast<uint32_t>(bitmapHeader->width);
		const auto stride = ((width + 3) & ~uint32_t{ 3 }) * pixelSize(pixelFormat);
		const auto height = static_cast<uint32_t>(bitmapHeader->height >= 0 ? bitmapHeader->height : -bitmapHeader->height);
		const auto data = reader.peek(size_t{ stride } * height);
		if (!data)
			return nullptr;

		info = { width, height, stride, pixelFormat, bitmapHeader->height >= 0 ? ImageAxes::XRightYUp : ImageAxes::XRightYDown };
		return data;
	}
}
