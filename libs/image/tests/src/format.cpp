// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "image.hpp"

#include <seir_data/blob.hpp>
#include <seir_data/writer.hpp>
#include <seir_data/buffer_writer.hpp>

#include <doctest/doctest.h>

namespace
{
	void checkSavedImage(const void* data, uint64_t size, const std::string& name)
	{
		const auto blob = seir::Blob::from(SEIR_TEST_DIR + name);
		REQUIRE(blob);
		REQUIRE(size == blob->size());
		CHECK(std::memcmp(data, blob->data(), blob->size()) == 0);
	}

	seir::Image loadImage(const std::string& name)
	{
		auto blob = seir::Blob::from(SEIR_TEST_DIR + name);
		REQUIRE(blob);
		auto image = seir::Image::load(blob);
		REQUIRE(image);
		return std::move(*image);
	}

	seir::Image makeColorImage(bool withAlpha, seir::ImageAxes axes, bool padding = false)
	{
		constexpr uint32_t width = 16;
		constexpr uint32_t height = 16;
		const uint32_t pixelSize = withAlpha ? 4u : 3u;
		const auto stride = (16 + static_cast<uint32_t>(padding)) * pixelSize;
		seir::Buffer buffer{ stride * height };
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
			if (padding)
				for (uint32_t i = 0; i < pixelSize; ++i)
					*data++ = 0xCC;
		}
		return { { width, height, stride, withAlpha ? seir::PixelFormat::Bgra32 : seir::PixelFormat::Bgr24, axes }, std::move(buffer) };
	}

	seir::Image makeGrayscaleImage(seir::ImageAxes axes, bool padding = false)
	{
		constexpr uint32_t width = 32;
		constexpr uint32_t height = 16;
		const auto stride = width + static_cast<uint32_t>(padding);
		seir::Buffer buffer{ stride * height };
		auto data = reinterpret_cast<uint8_t*>(buffer.data());
		for (uint32_t row = 0; row < height; ++row)
		{
			const auto y = axes == seir::ImageAxes::XRightYDown ? row : height - 1 - row;
			for (uint32_t x = 0; x < width; ++x)
			{
				constexpr auto halfHeight = height / 2;
				if (y < halfHeight)
					*data++ = static_cast<uint8_t>(x * 256 / width + y);
				else
					*data++ = static_cast<uint8_t>((y - halfHeight) * 256 / halfHeight + x);
			}
			if (padding)
				*data++ = 0xCC;
		}
		return { { width, height, stride, seir::PixelFormat::Gray8, axes }, std::move(buffer) };
	}
}

#if SEIR_IMAGE_BMP
TEST_CASE("BMP")
{
	const auto image = ::loadImage("bgr24_rd.bmp");
	CHECK(image == ::makeColorImage(false, seir::ImageAxes::XRightYDown));
}
#endif

#if SEIR_IMAGE_DDS
TEST_CASE("DDS")
{
	const auto image = ::loadImage("bgra32.dds");
	CHECK(image == ::makeColorImage(true, seir::ImageAxes::XRightYDown));
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
	SUBCASE("load")
	{
		SUBCASE("Gray8")
		{
			const auto jpegImage = ::loadImage("gray8_rd.jpg");
			const auto tgaImage = ::loadImage("gray8_rd.tga"); // JPEG is actually lossless in this case.
			CHECK(jpegImage == tgaImage);
		}
		SUBCASE("Bgr24")
		{
			const auto jpegImage = ::loadImage("bgr24_rd.jpg");
			const auto tgaImage = ::loadImage("bgr24_rd.jpg.tga");
			CHECK(jpegImage == tgaImage);
		}
	}
#	endif
	SUBCASE("save")
	{
		seir::Image image;
		std::string fileName;
		SUBCASE("Gray8")
		{
			fileName = "gray8_rd.jpg";
			SUBCASE("XRightYDown")
			{
				SUBCASE("without padding") { image = ::makeGrayscaleImage(seir::ImageAxes::XRightYDown); }
				SUBCASE("with padding") { image = ::makeGrayscaleImage(seir::ImageAxes::XRightYDown, true); }
			}
			SUBCASE("XRightYUp")
			{
				SUBCASE("without padding") { image = ::makeGrayscaleImage(seir::ImageAxes::XRightYUp); }
				SUBCASE("with padding") { image = ::makeGrayscaleImage(seir::ImageAxes::XRightYUp, true); }
			}
		}
		SUBCASE("Bgr24")
		{
			fileName = "bgr24_rd.jpg";
			SUBCASE("XRightYDown")
			{
				SUBCASE("without padding") { image = ::makeColorImage(false, seir::ImageAxes::XRightYDown); }
				SUBCASE("with padding") { image = ::makeColorImage(false, seir::ImageAxes::XRightYDown, true); }
			}
			SUBCASE("XRightYUp")
			{
				SUBCASE("without padding") { image = ::makeColorImage(false, seir::ImageAxes::XRightYUp); }
				SUBCASE("with padding") { image = ::makeColorImage(false, seir::ImageAxes::XRightYUp, true); }
			}
		}
		SUBCASE("Bgra24")
		{
			fileName = "bgr24_rd.jpg";
			SUBCASE("XRightYDown")
			{
				SUBCASE("without padding") { image = ::makeColorImage(true, seir::ImageAxes::XRightYDown); }
				SUBCASE("with padding") { image = ::makeColorImage(true, seir::ImageAxes::XRightYDown, true); }
			}
			SUBCASE("XRightYUp")
			{
				SUBCASE("without padding") { image = ::makeColorImage(true, seir::ImageAxes::XRightYUp); }
				SUBCASE("with padding") { image = ::makeColorImage(true, seir::ImageAxes::XRightYUp, true); }
			}
		}
		seir::Buffer buffer;
		seir::BufferWriter writer{ buffer };
		REQUIRE(image.save(seir::ImageFormat::Jpeg, writer, 0));
		::checkSavedImage(buffer.data(), writer.size(), fileName);
	}
}
#endif

#if SEIR_IMAGE_PNG
TEST_CASE("PNG")
{
	SUBCASE("load")
	{
		auto blob = seir::Blob::from(SEIR_TEST_DIR "rgb24.png");
		REQUIRE(blob);
		CHECK_FALSE(static_cast<bool>(seir::Image::load(blob)));
	}
	SUBCASE("save")
	{
		seir::Image image;
		SUBCASE("without padding") { image = ::makeColorImage(false, seir::ImageAxes::XRightYDown); }
		SUBCASE("with padding") { image = ::makeColorImage(false, seir::ImageAxes::XRightYDown, true); }
		seir::Buffer buffer;
		seir::BufferWriter writer{ buffer };
		REQUIRE(image.save(seir::ImageFormat::Png, writer, 0));
		::checkSavedImage(buffer.data(), writer.size(), "rgb24.png");
	}
}
#endif

#if SEIR_IMAGE_TGA
TEST_CASE("TGA")
{
	SUBCASE("load")
	{
		SUBCASE("Gray8")
		{
			const auto image = ::loadImage("gray8_rd.tga");
			CHECK(image == ::makeGrayscaleImage(seir::ImageAxes::XRightYDown));
		}
		SUBCASE("Bgr32")
		{
			const auto image = ::loadImage("bgr24_rd.tga");
			CHECK(image == ::makeColorImage(false, seir::ImageAxes::XRightYDown));
		}
		SUBCASE("Bgra32")
		{
			const auto image = ::loadImage("bgra32_rd.tga");
			CHECK(image == ::makeColorImage(true, seir::ImageAxes::XRightYDown));
		}
	}
	SUBCASE("save")
	{
		seir::Image image;
		std::string fileName;
		SUBCASE("Gray8")
		{
			fileName = "gray8_rd.tga";
			SUBCASE("padding = false") { image = ::makeGrayscaleImage(seir::ImageAxes::XRightYDown); }
			SUBCASE("padding = true") { image = ::makeGrayscaleImage(seir::ImageAxes::XRightYDown, true); }
		}
		SUBCASE("Bgr24")
		{
			fileName = "bgr24_rd.tga";
			SUBCASE("without padding") { image = ::makeColorImage(false, seir::ImageAxes::XRightYDown); }
			SUBCASE("with padding") { image = ::makeColorImage(false, seir::ImageAxes::XRightYDown, true); }
		}
		SUBCASE("Bgra32")
		{
			fileName = "bgra32_rd.tga";
			SUBCASE("without padding") { image = ::makeColorImage(true, seir::ImageAxes::XRightYDown); }
			SUBCASE("with padding") { image = ::makeColorImage(true, seir::ImageAxes::XRightYDown, true); }
		}
		seir::Buffer buffer;
		seir::BufferWriter writer{ buffer };
		REQUIRE(image.save(seir::ImageFormat::Tga, writer, 0));
		::checkSavedImage(buffer.data(), writer.size(), fileName);
	}
}
#endif
