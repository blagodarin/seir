// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "image.hpp"

#include <seir_data/blob.hpp>

#include <doctest/doctest.h>

namespace
{
	inline seir::Image makeColorImage(bool withAlpha, seir::ImageAxes axes)
	{
		seir::Buffer<std::byte> buffer{ static_cast<size_t>(16 * 16 * (withAlpha ? 4 : 3)) };
		auto data = reinterpret_cast<uint8_t*>(buffer.data());
		for (int row = 0; row < 16; ++row)
		{
			const auto y = axes == seir::ImageAxes::XRightYDown ? row : 15 - row;
			for (int x = 0; x < 16; ++x)
			{
				int b = 0;
				int g = 0;
				int r = 0;
				if (y < 4)
				{
					r = (y * 16 + x) * 4 + 3;
				}
				else if (y < 8)
				{
					r = ((7 - y) * 16 + x) * 4 + 3;
					g = ((y - 4) * 16 + x) * 4 + 3;
				}
				else if (y < 12)
				{
					g = ((11 - y) * 16 + x) * 4 + 3;
					b = ((y - 8) * 16 + x) * 4 + 3;
				}
				else
				{
					b = ((15 - y) * 16 + x) * 4 + 3;
				}
				*data++ = static_cast<uint8_t>(b);
				*data++ = static_cast<uint8_t>(g);
				*data++ = static_cast<uint8_t>(r);
				if (withAlpha)
					*data++ = static_cast<uint8_t>(x * 16 + 15);
			}
		}
		return { { 16, 16, withAlpha ? seir::PixelFormat::Bgra32 : seir::PixelFormat::Bgr24, axes }, std::move(buffer) };
	}
}

#if SEIR_IMAGE_BMP
TEST_CASE("BMP")
{
	const auto blob = seir::Blob::from(SEIR_TEST_DIR "bgr24_right_down.bmp");
	REQUIRE(blob);
	const auto image = seir::Image::load(blob);
	REQUIRE(image);
	CHECK(*image == ::makeColorImage(false, seir::ImageAxes::XRightYDown));
}
#endif

#if SEIR_IMAGE_ICO
TEST_CASE("ICO")
{
	const auto blob = seir::Blob::from(SEIR_TEST_DIR "bgra32_right_up.ico");
	REQUIRE(blob);
	const auto image = seir::Image::load(blob);
	REQUIRE(image);
	CHECK(*image == ::makeColorImage(true, seir::ImageAxes::XRightYUp));
}
#endif
