// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstdint>

namespace seir
{
	enum class BmpCompression : uint32_t
	{
		Rgb = 0, // See BI_RGB.
	};

#pragma pack(push, 1)

	// See BITMAPINFOHEADER.
	struct BmpImageHeader
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

	static_assert(sizeof(BmpImageHeader) == 40);
}
