// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/endian.hpp>
#include <seir_data/reader.hpp>
#include <seir_image/image.hpp>

namespace seir
{
	constexpr auto kBmpFileID = makeCC('B', 'M');
#if SEIR_IMAGE_BMP
	const void* loadBmpImage(Reader&, ImageInfo&) noexcept;
#endif

	constexpr auto kDdsFileID = makeCC('D', 'D', 'S', ' ');
#if SEIR_IMAGE_DDS
	const void* loadDdsImage(Reader&, ImageInfo&) noexcept;
#endif

#if SEIR_IMAGE_ICO
	const void* loadIcoImage(Reader&, ImageInfo&) noexcept;
#endif

	constexpr auto kJpegFileID = makeCC('\xff', '\xd8'); // SOI marker.
#if SEIR_IMAGE_JPEG
	const void* loadJpegImage(Reader&, ImageInfo&, Buffer&) noexcept;
	bool saveJpegImage(Writer&, const ImageInfo&, const void* data, int compressionLevel) noexcept;
#endif

	constexpr auto kPngFileID = makeCC('\x89', 'P', 'N', 'G', '\r', '\n', '\x1a', '\n');
#if SEIR_IMAGE_PNG
	bool savePngImage(Writer&, const ImageInfo&, const void* data, int compressionLevel) noexcept;
#endif

#if SEIR_IMAGE_TGA
	const void* loadTgaImage(Reader&, ImageInfo&) noexcept;
	bool saveTgaImage(Writer&, const ImageInfo&, const void* data) noexcept;
#endif
}
