// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "image.hpp"

#include <doctest/doctest.h>

static_assert(pixelSize(seir::PixelFormat::Gray8) == 1);
static_assert(pixelSize(seir::PixelFormat::Intensity8) == 1);
static_assert(pixelSize(seir::PixelFormat::GrayAlpha16) == 2);
static_assert(pixelSize(seir::PixelFormat::Rgb24) == 3);
static_assert(pixelSize(seir::PixelFormat::Bgr24) == 3);
static_assert(pixelSize(seir::PixelFormat::Rgba32) == 4);
static_assert(pixelSize(seir::PixelFormat::Bgra32) == 4);

TEST_CASE("ImageInfo")
{
	seir::ImageInfo info;
	CHECK(info.axes() == seir::ImageAxes::XRightYDown);
	CHECK(info.frameSize() == 0);
	CHECK(info.pixelFormat() == seir::PixelFormat::Gray8);
	CHECK(info.height() == 0);
	CHECK(info.width() == 0);
}

TEST_CASE("Image::Image()")
{
	seir::Image image;
	CHECK_FALSE(image.data());
	CHECK(image.info() == seir::ImageInfo{});
}
