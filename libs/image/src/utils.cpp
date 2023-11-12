// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_image/utils.hpp>

#include <seir_image/image.hpp>

#include <cstring>

namespace
{
	void copyImage_rgb24_bgr24(size_t width, size_t height, const uint8_t* src, ptrdiff_t srcStride, uint8_t* dst, ptrdiff_t dstStride) noexcept
	{
		const auto dst_row_size = width * 3;
		for (auto y = height; y > 0; --y)
		{
			for (size_t a = 0, b = 0; a < dst_row_size; a += 3, b += 3)
			{
				dst[a + 0] = src[b + 2];
				dst[a + 1] = src[b + 1];
				dst[a + 2] = src[b + 0];
			}
			src += srcStride;
			dst += dstStride;
		}
	}

	void copyImage_rgb24_bgra32(size_t width, size_t height, const uint8_t* src, ptrdiff_t srcStride, uint8_t* dst, ptrdiff_t dstStride) noexcept
	{
		const auto dstRowSize = width * 4;
		for (auto y = height; y > 0; --y)
		{
			for (size_t a = 0, b = 0; a < dstRowSize; a += 4, b += 3)
			{
				dst[a + 0] = src[b + 2];
				dst[a + 1] = src[b + 1];
				dst[a + 2] = src[b + 0];
				dst[a + 3] = 0xff;
			}
			src += srcStride;
			dst += dstStride;
		}
	}

	void copyImage_rgb24_rgba32(size_t width, size_t height, const uint8_t* src, ptrdiff_t srcStride, uint8_t* dst, ptrdiff_t dstStride) noexcept
	{
		const auto dstRowSize = width * 4;
		for (auto y = height; y > 0; --y)
		{
			for (size_t a = 0, b = 0; a < dstRowSize; a += 4, b += 3)
			{
				dst[a + 0] = src[b + 0];
				dst[a + 1] = src[b + 1];
				dst[a + 2] = src[b + 2];
				dst[a + 3] = 0xff;
			}
			src += srcStride;
			dst += dstStride;
		}
	}

	void copyImage_rgba32_bgra32(size_t width, size_t height, const uint8_t* src, ptrdiff_t srcStride, uint8_t* dst, ptrdiff_t dstStride) noexcept
	{
		const auto dstRowSize = width * 4;
		for (auto y = height; y > 0; --y)
		{
			for (size_t a = 0, b = 0; a < dstRowSize; a += 4, b += 4)
			{
				dst[a + 0] = src[b + 2];
				dst[a + 1] = src[b + 1];
				dst[a + 2] = src[b + 0];
				dst[a + 3] = src[b + 3];
			}
			src += srcStride;
			dst += dstStride;
		}
	}

	void copyImage_x8_x8(size_t width, size_t height, const uint8_t* src, ptrdiff_t srcStride, uint8_t* dst, ptrdiff_t dstStride) noexcept
	{
		for (auto y = height; y > 0; --y)
		{
			std::memcpy(dst, src, width);
			src += srcStride;
			dst += dstStride;
		}
	}

	void copyImage_x8_xxxa32(size_t width, size_t height, const uint8_t* src, ptrdiff_t srcStride, uint8_t* dst, ptrdiff_t dstStride) noexcept
	{
		const auto dstRowSize = width * 4;
		for (auto y = height; y > 0; --y)
		{
			for (size_t a = 0, b = 0; a < dstRowSize; a += 4, ++b)
			{
				dst[a + 0] = src[b + 0];
				dst[a + 1] = src[b + 0];
				dst[a + 2] = src[b + 0];
				dst[a + 3] = 0xff;
			}
			src += srcStride;
			dst += dstStride;
		}
	}

	void copyImage_x8_xxxx32(size_t width, size_t height, const uint8_t* src, ptrdiff_t srcStride, uint8_t* dst, ptrdiff_t dstStride) noexcept
	{
		const auto dstRowSize = width * 4;
		for (auto y = height; y > 0; --y)
		{
			for (size_t a = 0, b = 0; a < dstRowSize; a += 4, ++b)
			{
				dst[a + 0] = src[b + 0];
				dst[a + 1] = src[b + 0];
				dst[a + 2] = src[b + 0];
				dst[a + 3] = src[b + 0];
			}
			src += srcStride;
			dst += dstStride;
		}
	}

	void copyImage_xa16_xxxa32(size_t width, size_t height, const uint8_t* src, ptrdiff_t srcStride, uint8_t* dst, ptrdiff_t dstStride) noexcept
	{
		const auto dstRowSize = width * 4;
		for (auto y = height; y > 0; --y)
		{
			for (size_t a = 0, b = 0; a < dstRowSize; a += 4, b += 2)
			{
				dst[a + 0] = src[b + 0];
				dst[a + 1] = src[b + 0];
				dst[a + 2] = src[b + 0];
				dst[a + 3] = src[b + 1];
			}
			src += srcStride;
			dst += dstStride;
		}
	}
}

namespace seir
{
	bool copyImage(const ImageInfo& srcInfo, const void* srcData, const ImageInfo& dstInfo, void* dstData) noexcept
	{
		const auto width = srcInfo.width();
		const auto height = srcInfo.height();
		if (width != dstInfo.width() || height != dstInfo.height())
			return false;

		const auto src = static_cast<const uint8_t*>(srcData);
		const auto srcStride = static_cast<ptrdiff_t>(srcInfo.stride());
		const auto srcFormat = srcInfo.pixelFormat();

		auto dst = static_cast<uint8_t*>(dstData);
		auto dstStride = static_cast<ptrdiff_t>(dstInfo.stride());
		const auto dstFormat = dstInfo.pixelFormat();

		if (srcInfo.axes() != dstInfo.axes())
		{
			dst += static_cast<ptrdiff_t>(height - 1) * dstStride;
			dstStride = -dstStride;
		}

		if (srcFormat == dstFormat)
		{
			::copyImage_x8_x8(width * pixelSize(dstFormat), height, src, srcStride, dst, dstStride);
			return true;
		}

		switch (srcFormat)
		{
		case PixelFormat::Gray8:
			if (dstFormat == PixelFormat::Bgra32 || dstFormat == PixelFormat::Rgba32)
				::copyImage_x8_xxxa32(width, height, src, srcStride, dst, dstStride);
			else
				break;
			return true;

		case PixelFormat::Intensity8:
			if (dstFormat == PixelFormat::Bgra32 || dstFormat == PixelFormat::Rgba32)
				::copyImage_x8_xxxx32(width, height, src, srcStride, dst, dstStride);
			else
				break;
			return true;

		case PixelFormat::GrayAlpha16:
			if (dstFormat == PixelFormat::Bgra32 || dstFormat == PixelFormat::Rgba32)
				::copyImage_xa16_xxxa32(width, height, src, srcStride, dst, dstStride);
			else
				break;
			return true;

		case PixelFormat::Rgb24:
			if (dstFormat == PixelFormat::Bgr24)
				::copyImage_rgb24_bgr24(width, height, src, srcStride, dst, dstStride);
			else if (dstFormat == PixelFormat::Rgba32)
				::copyImage_rgb24_rgba32(width, height, src, srcStride, dst, dstStride);
			else if (dstFormat == PixelFormat::Bgra32)
				::copyImage_rgb24_bgra32(width, height, src, srcStride, dst, dstStride);
			else
				break;
			return true;

		case PixelFormat::Bgr24:
			if (dstFormat == PixelFormat::Rgb24)
				::copyImage_rgb24_bgr24(width, height, src, srcStride, dst, dstStride);
			else if (dstFormat == PixelFormat::Rgba32)
				::copyImage_rgb24_bgra32(width, height, src, srcStride, dst, dstStride);
			else if (dstFormat == PixelFormat::Bgra32)
				::copyImage_rgb24_rgba32(width, height, src, srcStride, dst, dstStride);
			else
				break;
			return true;

		case PixelFormat::Rgba32:
			if (dstFormat == PixelFormat::Bgra32)
				::copyImage_rgba32_bgra32(width, height, src, srcStride, dst, dstStride);
			else
				break;
			return true;

		case PixelFormat::Bgra32:
			if (dstFormat == PixelFormat::Rgba32)
				::copyImage_rgba32_bgra32(width, height, src, srcStride, dst, dstStride);
			else
				break;
			return true;
		}

		return false;
	}

	bool copyImage(const Image& src, const ImageInfo& dstInfo, void* dstData) noexcept
	{
		return copyImage(src.info(), src.data(), dstInfo, dstData);
	}
}
