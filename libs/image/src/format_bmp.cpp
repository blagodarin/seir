// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "format.hpp"

#include <seir_base/endian.hpp>

namespace
{
	enum class BmpCompression : uint32_t
	{
		Rgb = 0,
	};

#pragma pack(push, 1)

	struct BmpInfoHeader
	{
		uint32_t headerSize;
		int32_t width;
		int32_t height;
		uint16_t planes;
		uint16_t bitsPerPixel;
		BmpCompression compression;
		uint32_t imageSize;
		int32_t xPixelsPerMeter;
		int32_t yPixelsPerMeter;
		uint32_t usedColors;
		uint32_t requiredColors;
	};

#pragma pack(pop)

#if SEIR_IMAGE_BMP

	enum class BmpFileType : uint16_t
	{
		Bm = seir::makeCC('B', 'M'),
	};

#	pragma pack(push, 1)

	struct BmpFileHeader
	{
		BmpFileType fileType;
		uint32_t fileSize;
		uint32_t reserved;
		uint32_t dataOffset;
	};

#	pragma pack(pop)
#endif

#if SEIR_IMAGE_ICO

	enum class IcoFileType : uint16_t
	{
		Ico = 1,
		Cur = 2,
	};

#	pragma pack(push, 1)

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

#	pragma pack(pop)
#endif
}

namespace seir
{
#if SEIR_IMAGE_BMP
	const void* loadBmpImage(Reader& reader, ImageInfo& info)
	{
		const auto fileHeader = reader.read<BmpFileHeader>();
		if (!fileHeader
			|| fileHeader->fileType != BmpFileType::Bm
			|| fileHeader->reserved != 0)
			return nullptr;

		const auto bitmapHeader = reader.read<BmpInfoHeader>();
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
#endif

#if SEIR_IMAGE_ICO
	const void* loadIcoImage(Reader& reader, ImageInfo& info)
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
		if (const auto bitmapHeader = reader.read<BmpInfoHeader>(); !bitmapHeader
			|| bitmapHeader->headerSize < sizeof *bitmapHeader
			|| bitmapHeader->width != width
			|| bitmapHeader->height != height * 2
			|| bitmapHeader->planes != 1
			|| bitmapHeader->bitsPerPixel != imageHeader->ico.bitsPerPixel
			|| bitmapHeader->compression != BmpCompression::Rgb)
			return nullptr;

		constexpr auto pixelFormat = PixelFormat::Bgra32;
		const auto stride = ((width + 3) & ~uint32_t{ 3 }) * pixelSize(pixelFormat);
		const auto data = reader.peek(size_t{ stride } * height);
		if (!data)
			return nullptr;

		info = { width, height, stride, pixelFormat, ImageAxes::XRightYUp };
		return data;
	}
#endif
}
