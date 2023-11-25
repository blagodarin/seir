// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "format.hpp"

#include <limits>

namespace
{
	enum : uint32_t
	{
		DDSD_CAPS = 0x1,
		DDSD_HEIGHT = 0x2,
		DDSD_WIDTH = 0x4,
		DDSD_PITCH = 0x8,
		DDSD_PIXELFORMAT = 0x1000,
		DDSD_MIPMAPCOUNT = 0x20000,
		DDSD_LINEARSIZE = 0x80000,
		DDSD_DEPTH = 0x800000,
	};

	// DDS documentation advises not to check DDSD_CAPS and DDSD_PIXELFORMAT being set.
	constexpr auto kDdsRequiredFlags = DDSD_HEIGHT | DDSD_WIDTH;
	constexpr auto kDdsUnsupportedFlags = ~(kDdsRequiredFlags | DDSD_CAPS | DDSD_PITCH | DDSD_PIXELFORMAT);

	enum : uint32_t
	{
		DDSCAPS_COMPLEX = 0x8,
		DDSCAPS_TEXTURE = 0x1000,
		DDSCAPS_MIPMAP = 0x400000,
	};

	// DDS documentation advises not to check DDSCAPS_TEXTURE being set.
	constexpr auto kDdsUnsupportedCaps = ~DDSCAPS_TEXTURE;

	enum : uint32_t
	{
		DDPF_ALPHAPIXELS = 0x1,
		DDPF_ALPHA = 0x2,
		DDPF_FOURCC = 0x4,
		DDPF_RGB = 0x40,
		DDPF_YUV = 0x200,
		DDPF_LUMINANCE = 0x20000,
	};

#pragma pack(push, 1)

	struct DdsPixelFormat
	{
		uint32_t size;
		uint32_t flags;
		uint32_t fourcc;
		uint32_t bits;
		uint32_t red;
		uint32_t green;
		uint32_t blue;
		uint32_t alpha;
	};

	struct DdsHeader
	{
		uint32_t magic;
		uint32_t headerSize;
		uint32_t flags;
		uint32_t height;
		uint32_t width;
		uint32_t pitchOrLinearSize;
		uint32_t depth;
		uint32_t mipmapCount;
		uint32_t reserved1[11];
		DdsPixelFormat format;
		uint32_t caps;
		uint32_t caps2;
		uint32_t caps3;
		uint32_t caps4;
		uint32_t reserved2;
	};

#pragma pack(pop)

	constexpr uint32_t kDdsPixelFormatSize = 32;
	static_assert(sizeof(DdsPixelFormat) == kDdsPixelFormatSize);

	constexpr uint32_t kDdsHeaderSize = 124;
	static_assert(sizeof(DdsHeader) == sizeof seir::kDdsFileID + kDdsHeaderSize);
}

namespace seir
{
	const void* loadDdsImage(Reader& reader, ImageInfo& info) noexcept
	{
		const auto header = reader.read<DdsHeader>();
		if (!header
			|| header->magic != kDdsFileID
			|| header->headerSize != kDdsHeaderSize
			|| (header->flags & kDdsRequiredFlags) != kDdsRequiredFlags
			|| (header->flags & kDdsUnsupportedFlags)
			|| !header->height
			|| !header->width
			|| header->depth
			|| header->mipmapCount
			|| header->reserved1[0]
			|| header->reserved1[1]
			|| header->reserved1[2]
			|| header->reserved1[3]
			|| header->reserved1[4]
			|| header->reserved1[5]
			|| header->reserved1[6]
			|| header->reserved1[7]
			|| header->reserved1[8]
			|| header->reserved1[9]
			|| header->reserved1[10]
			|| header->format.size != kDdsPixelFormatSize
			|| header->format.fourcc
			|| (header->caps & kDdsUnsupportedCaps)
			|| header->caps2
			|| header->caps3
			|| header->caps4
			|| header->reserved2)
			return nullptr;

		PixelFormat pixelFormat;
		switch (header->format.flags)
		{
		case DDPF_RGB:
			if (header->format.bits != 24 || header->format.green != 0xff00 || header->format.alpha)
				return nullptr;
			if (header->format.red == 0xff0000 && header->format.blue == 0xff)
				pixelFormat = PixelFormat::Bgr24;
			else if (header->format.red == 0xff && header->format.blue == 0xff0000)
				pixelFormat = PixelFormat::Rgb24;
			else
				return nullptr;
			break;

		case DDPF_RGB | DDPF_ALPHAPIXELS:
			if (header->format.bits != 32 || header->format.green != 0xff00 || header->format.alpha != 0xff000000)
				return nullptr;
			if (header->format.red == 0xff0000 && header->format.blue == 0xff)
				pixelFormat = PixelFormat::Bgra32;
			else if (header->format.red == 0xff && header->format.blue == 0xff0000)
				pixelFormat = PixelFormat::Rgba32;
			else
				return nullptr;
			break;

		case DDPF_LUMINANCE:
			if (header->format.bits != 8)
				return nullptr;
			if (header->format.red == 0xff && !header->format.green && !header->format.blue && !header->format.alpha)
				pixelFormat = PixelFormat::Gray8;
			else
				return nullptr;
			break;

		case DDPF_LUMINANCE | DDPF_ALPHAPIXELS:
			if (header->format.bits != 16)
				return nullptr;
			if (header->format.red == 0xff && !header->format.green && !header->format.blue && header->format.alpha == 0xff00)
				pixelFormat = PixelFormat::GrayAlpha16;
			else
				return nullptr;
			break;

		default:
			return nullptr;
		}

		const auto pixelBytes = pixelSize(pixelFormat);
		if (header->width > std::numeric_limits<uint32_t>::max() / pixelBytes)
			return nullptr;

		auto stride = header->width * pixelBytes;
		if (header->flags & DDSD_PITCH)
		{
			if (header->pitchOrLinearSize < stride)
				return nullptr;
			stride = header->pitchOrLinearSize;
		}

		if (header->height > std::numeric_limits<uint32_t>::max() / stride)
			return nullptr;

		const auto data = reader.peek(stride * header->height);
		if (!data)
			return nullptr;

		info = { header->width, header->height, stride, pixelFormat, ImageAxes::XRightYDown };
		return data;
	}
}
