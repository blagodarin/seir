// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

namespace seir
{
	class Image;
	class ImageInfo;

	//
	bool copyImage(const ImageInfo& srcInfo, const void* srcData, const ImageInfo& dstInfo, void* dstData) noexcept;
	bool copyImage(const Image& src, const ImageInfo& dstInfo, void* dstData) noexcept;
}
