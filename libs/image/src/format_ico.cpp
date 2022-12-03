// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "format.hpp"

#include "bmp.hpp"

namespace
{
	enum class IcoFileType : uint16_t
	{
		Ico = 1,
		Cur = 2,
	};

#pragma pack(push, 1)

	struct IcoFileHeader
	{
		uint16_t reserved;
		IcoFileType type;
		uint16_t count;
	};

	struct IcoImageHeader
	{
		uint8_t width;
		uint8_t height;
		uint8_t colorCount;
		uint8_t reserved;
		union
		{
			struct
			{
				uint16_t colorPlanes;
				uint16_t bitsPerPixel;
			} ico;
			struct
			{
				uint16_t hotspotLeft;
				uint16_t hotspotLop;
			} cur;
		};
		uint32_t dataSize;
		uint32_t dataOffset;
	};

#pragma pack(pop)
}

namespace seir
{
	const void* loadIcoImage(Reader& reader, ImageInfo& info) noexcept
	{
		if (const auto fileHeader = reader.read<IcoFileHeader>(); !fileHeader
			|| fileHeader->reserved != 0
			|| fileHeader->type != IcoFileType::Ico
			|| fileHeader->count != 1)
			return nullptr;

		const auto imageHeader = reader.read<IcoImageHeader>();
		if (!imageHeader
			|| imageHeader->colorCount != 0
			|| imageHeader->reserved != 0
			|| imageHeader->ico.colorPlanes != 1
			|| imageHeader->ico.bitsPerPixel != 32
			|| !reader.seek(imageHeader->dataOffset))
			return nullptr;

		const uint16_t width = imageHeader->width ? imageHeader->width : 256u;
		const uint16_t height = imageHeader->height ? imageHeader->height : 256u;
		if (const auto bitmapHeader = reader.read<BmpImageHeader>(); !bitmapHeader
			|| bitmapHeader->headerSize < sizeof *bitmapHeader
			|| bitmapHeader->width != width
			|| bitmapHeader->height != height * 2
			|| bitmapHeader->planes != 1
			|| bitmapHeader->bitsPerPixel != imageHeader->ico.bitsPerPixel
			|| bitmapHeader->compression != BmpCompression::Rgb)
			return nullptr;

		constexpr auto pixelFormat = PixelFormat::Bgra32;
		const auto stride = ((width + 3) & ~uint32_t{ 3 }) * pixelSize(pixelFormat);
		const auto data = reader.peek(size_t{ stride } * height); // cppcheck-suppress[redundantAssignment]
		if (!data)
			return nullptr;

		info = { width, height, stride, pixelFormat, ImageAxes::XRightYUp };
		return data;
	}
}
