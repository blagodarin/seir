// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/buffer.hpp>
#include <seir_base/shared_ptr.hpp>

#include <cstdint>
#include <optional>

namespace seir
{
	class Blob;
	class Writer;

	// Pixel format.
	enum class PixelFormat
	{
		Gray8 = 0b1'0,        // Gray.
		Intensity8 = 0b1'1,   // Single channel for both grayscale and alpha.
		GrayAlpha16 = 0b10'0, // Gray-alpha.
		Rgb24 = 0b11'0,       // Red-green-blue.
		Bgr24 = 0b11'1,       // Blue-green-red (reverse).
		Rgba32 = 0b100'0,     // Red-green-blue-alpha.
		Bgra32 = 0b100'1,     // Blue-green-red-alpha (reverse RGB).
	};

	//
	[[nodiscard]] constexpr uint32_t pixelSize(PixelFormat format) noexcept { return static_cast<uint32_t>(format) >> 1; }

	// Image axes orientation.
	enum class ImageAxes
	{
		XRightYDown, // X is left-to-right, Y is top-to-bottom.
		XRightYUp,   // X is left-to-right, Y is bottom-to-top.
	};

	//
	class ImageInfo
	{
	public:
		constexpr ImageInfo() noexcept = default;
		constexpr ImageInfo(uint32_t width, uint32_t height, uint32_t stride, PixelFormat pixelFormat, ImageAxes axes = ImageAxes::XRightYDown) noexcept
			: _width{ width }, _height{ height }, _stride{ stride }, _pixelFormat{ pixelFormat }, _axes{ axes } {}
		constexpr ImageInfo(uint32_t width, uint32_t height, PixelFormat pixelFormat, ImageAxes axes = ImageAxes::XRightYDown) noexcept
			: ImageInfo{ width, height, width * seir::pixelSize(pixelFormat), pixelFormat, axes } {}

		[[nodiscard]] constexpr ImageAxes axes() const noexcept { return _axes; }
		[[nodiscard]] constexpr uint32_t frameSize() const noexcept { return _stride * _height; }
		[[nodiscard]] constexpr uint32_t height() const noexcept { return _height; }
		[[nodiscard]] constexpr PixelFormat pixelFormat() const noexcept { return _pixelFormat; }
		[[nodiscard]] constexpr uint32_t pixelSize() const noexcept { return seir::pixelSize(_pixelFormat); }
		[[nodiscard]] constexpr uint32_t stride() const noexcept { return _stride; }
		[[nodiscard]] constexpr uint32_t width() const noexcept { return _width; }

	private:
		uint32_t _width = 0;
		uint32_t _height = 0;
		uint32_t _stride = 0;
		PixelFormat _pixelFormat = PixelFormat::Gray8;
		ImageAxes _axes = ImageAxes::XRightYDown;
	};

	// Supported formats for saving images.
	enum class ImageFormat
	{
		Tga,  // Truevision TARGA (TGA) file format.
		Jpeg, // Joint Photographic Experts Group (JPEG) file format.
		Png,  // Portable Network Graphics (PNG) file format.
	};

	//
	class Image
	{
	public:
		//
		[[nodiscard]] static std::optional<Image> load(const SharedPtr<Blob>&);

		Image() noexcept;
		Image(const Image&) = delete;
		Image(Image&&) noexcept;
		Image& operator=(Image&) = delete;
		Image& operator=(Image&&) noexcept;
		~Image() noexcept;

		//
		Image(const ImageInfo&, Buffer&&) noexcept;

		//
		[[nodiscard]] const void* data() const noexcept { return _data; }

		//
		[[nodiscard]] const ImageInfo& info() const noexcept { return _info; }

		//
		bool save(ImageFormat, Writer&, int compressionLevel) const noexcept;

	private:
		ImageInfo _info;
		const void* _data = nullptr;
		SharedPtr<Blob> _blob; // If we managed to memory-map image contents...
		Buffer _buffer;        // ...and if we didn't.
	};
}
