// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "image.hpp"

#include <seir_image/utils.hpp>

#include <array>

#include <doctest/doctest.h>

namespace
{
	seir::Image makeImage(const seir::ImageInfo& info, const void* data)
	{
		seir::Buffer buffer{ info.frameSize() };
		std::memcpy(buffer.data(), data, info.frameSize());
		return { info, std::move(buffer) };
	}
}

TEST_CASE("copyImage")
{
	const auto check = [](const seir::Image& input, const seir::Image& expected) {
		seir::Buffer buffer{ expected.info().frameSize() };
		REQUIRE(seir::copyImage(input, expected.info(), buffer.data()));
		const seir::Image output{ expected.info(), std::move(buffer) };
		CHECK(output == expected);
	};

	SUBCASE("Gray8 to Bgra32")
	{
		const std::array<uint8_t, 8> gray{
			0x00, 0x01, 0x02, 0x03,
			0x10, 0x11, 0x12, 0x13
		};

		const std::array<uint8_t, 24> bgra{
			0x00, 0x00, 0x00, 0xff, 0x01, 0x01, 0x01, 0xff, 0x02, 0x02, 0x02, 0xff,
			0x10, 0x10, 0x10, 0xff, 0x11, 0x11, 0x11, 0xff, 0x12, 0x12, 0x12, 0xff
		};

		check(
			::makeImage({ 3, 2, 4, seir::PixelFormat::Gray8 }, gray.data()),
			::makeImage({ 3, 2, seir::PixelFormat::Bgra32 }, bgra.data()));
	}
	SUBCASE("Intensity8 to Bgra32")
	{
		const std::array<uint8_t, 8> intensity{
			0x00, 0x01, 0x02, 0x03,
			0x10, 0x11, 0x12, 0x13
		};

		const std::array<uint8_t, 24> bgra{
			0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02,
			0x10, 0x10, 0x10, 0x10, 0x11, 0x11, 0x11, 0x11, 0x12, 0x12, 0x12, 0x12
		};

		check(
			::makeImage({ 3, 2, 4, seir::PixelFormat::Intensity8 }, intensity.data()),
			::makeImage({ 3, 2, seir::PixelFormat::Bgra32 }, bgra.data()));
	}
	SUBCASE("GrayAlpha16 to Bgra32")
	{
		const std::array<uint8_t, 16> grayAlpha{
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
			0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17
		};

		const std::array<uint8_t, 24> bgra{
			0x00, 0x00, 0x00, 0x01, 0x02, 0x02, 0x02, 0x03, 0x04, 0x04, 0x04, 0x05,
			0x10, 0x10, 0x10, 0x11, 0x12, 0x12, 0x12, 0x13, 0x14, 0x14, 0x14, 0x15
		};

		check(
			::makeImage({ 3, 2, 8, seir::PixelFormat::GrayAlpha16 }, grayAlpha.data()),
			::makeImage({ 3, 2, seir::PixelFormat::Bgra32 }, bgra.data()));
	}
	SUBCASE("Rgb24 to Bgra32")
	{
		const std::array<uint8_t, 24> rgb{
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
			0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b
		};

		const std::array<uint8_t, 24> bgra{
			0x02, 0x01, 0x00, 0xff, 0x05, 0x04, 0x03, 0xff, 0x08, 0x07, 0x06, 0xff,
			0x12, 0x11, 0x10, 0xff, 0x15, 0x14, 0x13, 0xff, 0x18, 0x17, 0x16, 0xff
		};

		check(
			::makeImage({ 3, 2, 12, seir::PixelFormat::Rgb24 }, rgb.data()),
			::makeImage({ 3, 2, seir::PixelFormat::Bgra32 }, bgra.data()));
	}
	SUBCASE("Bgr24 to Bgra32")
	{
		const std::array<uint8_t, 24> bgr{
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
			0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b
		};

		const std::array<uint8_t, 24> bgra{
			0x00, 0x01, 0x02, 0xff, 0x03, 0x04, 0x05, 0xff, 0x06, 0x07, 0x08, 0xff,
			0x10, 0x11, 0x12, 0xff, 0x13, 0x14, 0x15, 0xff, 0x16, 0x17, 0x18, 0xff
		};

		check(
			::makeImage({ 3, 2, 12, seir::PixelFormat::Bgr24 }, bgr.data()),
			::makeImage({ 3, 2, seir::PixelFormat::Bgra32 }, bgra.data()));
	}
	SUBCASE("Rgba32 to Bgra32")
	{
		const std::array<uint8_t, 32> rgba{
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
			0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
		};

		const std::array<uint8_t, 24> bgra{
			0x02, 0x01, 0x00, 0x03, 0x06, 0x05, 0x04, 0x07, 0x0a, 0x09, 0x08, 0x0b,
			0x12, 0x11, 0x10, 0x13, 0x16, 0x15, 0x14, 0x17, 0x1a, 0x19, 0x18, 0x1b
		};

		check(
			::makeImage({ 3, 2, 16, seir::PixelFormat::Rgba32 }, rgba.data()),
			::makeImage({ 3, 2, seir::PixelFormat::Bgra32 }, bgra.data()));
	}
	SUBCASE("Bgra32 to Bgra32")
	{
		const std::array<uint8_t, 32> bgraWithStride{
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
			0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
		};

		const std::array<uint8_t, 24> bgra{
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
			0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b
		};

		check(
			::makeImage({ 3, 2, 16, seir::PixelFormat::Bgra32 }, bgraWithStride.data()),
			::makeImage({ 3, 2, seir::PixelFormat::Bgra32 }, bgra.data()));
	}
}

TEST_CASE("copyImage = false")
{
	SUBCASE("PixelFormat")
	{
		const auto check = [](seir::PixelFormat src, seir::PixelFormat dst) {
			const seir::ImageInfo srcInfo{ 1, 1, src };
			const seir::ImageInfo dstInfo{ 1, 1, dst };
			CHECK_FALSE(seir::copyImage(srcInfo, nullptr, dstInfo, nullptr));
		};

		check(seir::PixelFormat::Gray8, seir::PixelFormat::Intensity8);
		check(seir::PixelFormat::Gray8, seir::PixelFormat::GrayAlpha16);
		check(seir::PixelFormat::Gray8, seir::PixelFormat::Rgb24);
		check(seir::PixelFormat::Gray8, seir::PixelFormat::Bgr24);

		check(seir::PixelFormat::Intensity8, seir::PixelFormat::Gray8);
		check(seir::PixelFormat::Intensity8, seir::PixelFormat::GrayAlpha16);
		check(seir::PixelFormat::Intensity8, seir::PixelFormat::Rgb24);
		check(seir::PixelFormat::Intensity8, seir::PixelFormat::Bgr24);

		check(seir::PixelFormat::GrayAlpha16, seir::PixelFormat::Gray8);
		check(seir::PixelFormat::GrayAlpha16, seir::PixelFormat::Intensity8);
		check(seir::PixelFormat::GrayAlpha16, seir::PixelFormat::Rgb24);
		check(seir::PixelFormat::GrayAlpha16, seir::PixelFormat::Bgr24);

		check(seir::PixelFormat::Rgb24, seir::PixelFormat::Gray8);
		check(seir::PixelFormat::Rgb24, seir::PixelFormat::Intensity8);
		check(seir::PixelFormat::Rgb24, seir::PixelFormat::GrayAlpha16);

		check(seir::PixelFormat::Bgr24, seir::PixelFormat::Gray8);
		check(seir::PixelFormat::Bgr24, seir::PixelFormat::Intensity8);
		check(seir::PixelFormat::Bgr24, seir::PixelFormat::GrayAlpha16);

		check(seir::PixelFormat::Rgba32, seir::PixelFormat::Gray8);
		check(seir::PixelFormat::Rgba32, seir::PixelFormat::Intensity8);
		check(seir::PixelFormat::Rgba32, seir::PixelFormat::GrayAlpha16);
		check(seir::PixelFormat::Rgba32, seir::PixelFormat::Rgb24);
		check(seir::PixelFormat::Rgba32, seir::PixelFormat::Bgr24);

		check(seir::PixelFormat::Bgra32, seir::PixelFormat::Gray8);
		check(seir::PixelFormat::Bgra32, seir::PixelFormat::Intensity8);
		check(seir::PixelFormat::Bgra32, seir::PixelFormat::GrayAlpha16);
		check(seir::PixelFormat::Bgra32, seir::PixelFormat::Rgb24);
		check(seir::PixelFormat::Bgra32, seir::PixelFormat::Bgr24);
	}
}
