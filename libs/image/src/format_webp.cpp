// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "format.hpp"

#include <cassert>

#include <webp/decode.h>

namespace seir
{
	const void* loadWebpImage(const Reader& reader, ImageInfo& info, Buffer& buffer) noexcept
	{
		::WebPDecoderConfig config;
		if (::WebPInitDecoderConfig(&config))
		{
			const auto data = static_cast<const uint8_t*>(reader.peek(0));
			const auto size = reader.size() - reader.offset();
			if (::WebPGetFeatures(data, size, &config.input) == VP8_STATUS_OK
				&& !config.input.has_animation)
			{
				assert(config.input.width >= 0 && config.input.height >= 0);
				info = { static_cast<uint32_t>(config.input.width), static_cast<uint32_t>(config.input.height), PixelFormat::Bgra32 };
				if (buffer.tryReserve(info.frameSize(), 0))
				{
					config.output.colorspace = MODE_BGRA;
					config.output.is_external_memory = 1; // 2 means "slow memory".
					config.output.u.RGBA.rgba = reinterpret_cast<uint8_t*>(buffer.data());
					config.output.u.RGBA.stride = static_cast<int>(info.stride());
					config.output.u.RGBA.size = buffer.capacity();
					if (::WebPDecode(data, size, &config) == VP8_STATUS_OK)
						return buffer.data();
				}
			}
		}
		return nullptr;
	}
}
