// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_data/reader.hpp>
#include <seir_image/image.hpp>

namespace seir
{
	class ImageInfo;
	class Reader;

#if SEIR_IMAGE_BMP
	const void* loadBmpImage(Reader&, ImageInfo&) noexcept;
#endif
#if SEIR_IMAGE_ICO
	const void* loadIcoImage(Reader&, ImageInfo&) noexcept;
#endif
#if SEIR_IMAGE_JPEG
	const void* loadJpegImage(Reader&, ImageInfo&, Buffer<std::byte>&) noexcept;
	bool saveJpegImage(Writer&, const ImageInfo&, const void* data, int compressionLevel) noexcept;
#endif
#if SEIR_IMAGE_TGA
	const void* loadTgaImage(Reader&, ImageInfo&) noexcept;
	bool saveTgaImage(Writer&, const ImageInfo&, const void* data) noexcept;
#endif
}
