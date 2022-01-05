// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "image.hpp"

#include <seir_data/blob.hpp>

#include <doctest/doctest.h>

namespace
{
	seir::Image makeColorImage(bool withAlpha, seir::ImageAxes axes)
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

	seir::Image makeGrayscaleImage()
	{
		constexpr uint32_t kSize = 16;
		seir::Buffer<std::byte> buffer{ kSize * kSize };
		auto data = reinterpret_cast<uint8_t*>(buffer.data());
		for (uint32_t y = 0; y < kSize; ++y)
			for (uint32_t x = 0; x < kSize; ++x)
				*data++ = static_cast<uint8_t>(y < kSize / 2 ? x * kSize + y / 2 : (kSize - 1 - x) * kSize + (kSize - 1 - y) / 2);
		return { { kSize, kSize, seir::PixelFormat::Gray8, seir::ImageAxes::XRightYDown }, std::move(buffer) };
	}
}

#if SEIR_IMAGE_BMP
TEST_CASE("BMP")
{
	const auto blob = seir::Blob::from(SEIR_TEST_DIR "bgr24_rd.bmp");
	REQUIRE(blob);
	const auto image = seir::Image::load(blob);
	REQUIRE(image);
	CHECK(*image == ::makeColorImage(false, seir::ImageAxes::XRightYDown));
}
#endif

#if SEIR_IMAGE_ICO
TEST_CASE("ICO")
{
	const auto blob = seir::Blob::from(SEIR_TEST_DIR "bgra32_ru.ico");
	REQUIRE(blob);
	const auto image = seir::Image::load(blob);
	REQUIRE(image);
	CHECK(*image == ::makeColorImage(true, seir::ImageAxes::XRightYUp));
}
#endif

#if SEIR_IMAGE_TGA
TEST_CASE("TGA")
{
	SUBCASE("load gray8")
	{
		const auto blob = seir::Blob::from(SEIR_TEST_DIR "gray8_rd.tga");
		REQUIRE(blob);
		const auto image = seir::Image::load(blob);
		REQUIRE(image);
		CHECK(*image == ::makeGrayscaleImage());
	}
	SUBCASE("load bgr32")
	{
		const auto blob = seir::Blob::from(SEIR_TEST_DIR "bgr24_rd.tga");
		REQUIRE(blob);
		const auto image = seir::Image::load(blob);
		REQUIRE(image);
		CHECK(*image == ::makeColorImage(false, seir::ImageAxes::XRightYDown));
	}
	SUBCASE("load bgra32")
	{
		const auto blob = seir::Blob::from(SEIR_TEST_DIR "bgra32_rd.tga");
		REQUIRE(blob);
		const auto image = seir::Image::load(blob);
		REQUIRE(image);
		CHECK(*image == ::makeColorImage(true, seir::ImageAxes::XRightYDown));
	}
}
#endif
