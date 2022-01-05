// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "format.hpp"

#include <seir_data/reader.hpp>
#include <seir_image/image.hpp>

namespace
{
	enum class TgaColorMapType : uint8_t
	{
		None = 0,
		ColorMap = 1,
	};

	enum class TgaImageType : uint8_t
	{
		None = 0,
		ColorMapped = 1,
		TrueColor = 2,
		BlackAndWhite = 3,
		ColorMappedRle = 9,
		TrueColorRle = 10,
		BlackAndWhiteRle = 11,
	};

	enum TgaImageDescriptor : uint8_t
	{
		kTgaAlphaMask = 0x0F,

		kTgaOriginMask = 0x30,
		kTgaBottomLeft = 0x00,
		kTgaBottomRight = 0x10,
		kTgaTopLeft = 0x20,
		kTgaTopRight = 0x30,

		kTgaReservedMask = 0xC0,
	};

	enum TgaRlePacketHeader : uint8_t
	{
		kTgaRlePacketTypeMask = 0x80,
		kTgaRawPacket = 0x00,
		kTgaRunLengthPacket = 0x80,

		kTgaRlePixelCountMask = 0x7F,
	};

#pragma pack(push, 1)

	struct TgaHeader
	{
		uint8_t idLength;
		TgaColorMapType colorMapType;
		TgaImageType imageType;
		struct
		{
			uint16_t firstEntryIndex;
			uint16_t length;
			uint8_t entrySize;
		} colorMap;
		struct
		{
			uint16_t x;
			uint16_t y;
			uint16_t width;
			uint16_t height;
			uint8_t pixelDepth;
			uint8_t descriptor;
		} image;
	};

#pragma pack(pop)
}

namespace seir
{
	const void* loadTgaImage(Reader& reader, ImageInfo& info)
	{
		const auto header = reader.read<TgaHeader>();
		if (!header
			|| header->colorMapType != TgaColorMapType::None
			|| !header->image.width
			|| !header->image.height
			|| header->image.descriptor & kTgaReservedMask)
			return nullptr;

		PixelFormat pixelFormat;
		if (header->imageType == TgaImageType::BlackAndWhite)
		{
			if (header->image.pixelDepth == 8)
				pixelFormat = PixelFormat::Gray8;
			else
				return nullptr;
		}
		else if (header->imageType == TgaImageType::TrueColor)
		{
			if (const auto alpha = header->image.descriptor & kTgaAlphaMask; !alpha && header->image.pixelDepth == 24)
				pixelFormat = PixelFormat::Bgr24;
			else if (alpha == 8 && header->image.pixelDepth == 32)
				pixelFormat = PixelFormat::Bgra32;
			else
				return nullptr;
		}
		else
			return nullptr;

		ImageAxes axes;
		if (const auto origin = header->image.descriptor & kTgaOriginMask; origin == kTgaBottomLeft)
			axes = ImageAxes::XRightYUp;
		else if (origin == kTgaTopLeft)
			axes = ImageAxes::XRightYDown;
		else
			return nullptr;

		if (!reader.skip(header->idLength + header->colorMap.length * ((header->colorMap.entrySize + 7u) / 8u)))
			return nullptr;

		const auto stride = header->image.width * pixelSize(pixelFormat);
		const auto data = reader.peek(size_t{ stride } * header->image.height);
		if (!data)
			return nullptr;

		info = { header->image.width, header->image.height, stride, pixelFormat, axes };
		return data;
	}
}
