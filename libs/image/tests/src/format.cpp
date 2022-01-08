// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "image.hpp"

#include <seir_data/blob.hpp>
#include <seir_data/writer.hpp>

#include <doctest/doctest.h>

namespace
{
	seir::Image loadImage(const std::string& name)
	{
		auto blob = seir::Blob::from(SEIR_TEST_DIR + name);
		REQUIRE(blob);
		auto image = seir::Image::load(blob);
		REQUIRE(image);
		return std::move(*image);
	}

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
	const auto image = ::loadImage("bgr24_rd.bmp");
	CHECK(image == ::makeColorImage(false, seir::ImageAxes::XRightYDown));
}
#endif

#if SEIR_IMAGE_ICO
TEST_CASE("ICO")
{
	const auto image = ::loadImage("bgra32_ru.ico");
	CHECK(image == ::makeColorImage(true, seir::ImageAxes::XRightYUp));
}
#endif

#if SEIR_IMAGE_JPEG
TEST_CASE("JPEG")
{
#	if SEIR_IMAGE_TGA
	SUBCASE("load bgr24")
	{
		const auto jpegImage = ::loadImage("bgr24_rd.jpg");
		const auto tgaImage = ::loadImage("bgr24_rd.jpg.tga");
		CHECK(jpegImage == tgaImage);
	}
#	endif
	SUBCASE("save bgr24")
	{
		seir::Buffer<std::byte> buffer;
		const auto writer = seir::Writer::create(buffer);
		REQUIRE(writer);
		SUBCASE("ImageAxes::XRightYDown")
		{
			const auto image = ::makeColorImage(false, seir::ImageAxes::XRightYDown);
			REQUIRE(image.saveJpeg(*writer, 100));
		}
		SUBCASE("ImageAxes::XRightYUp")
		{
			const auto image = ::makeColorImage(false, seir::ImageAxes::XRightYUp);
			REQUIRE(image.saveJpeg(*writer, 100));
		}
		const auto blob = seir::Blob::from(SEIR_TEST_DIR "bgr24_rd.jpg");
		REQUIRE(blob);
		REQUIRE(writer->size() == blob->size());
		CHECK(std::memcmp(buffer.data(), blob->data(), blob->size()) == 0);
	}
}
#endif

#if SEIR_IMAGE_TGA
TEST_CASE("TGA")
{
	SUBCASE("load gray8")
	{
		const auto image = ::loadImage("gray8_rd.tga");
		CHECK(image == ::makeGrayscaleImage());
	}
	SUBCASE("load bgr32")
	{
		const auto image = ::loadImage("bgr24_rd.tga");
		CHECK(image == ::makeColorImage(false, seir::ImageAxes::XRightYDown));
	}
	SUBCASE("load bgra32")
	{
		const auto image = ::loadImage("bgra32_rd.tga");
		CHECK(image == ::makeColorImage(true, seir::ImageAxes::XRightYDown));
	}
}
#endif
