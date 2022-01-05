// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_image/image.hpp>

#include <cstring>

namespace seir
{
	constexpr bool operator==(const ImageInfo& a, const ImageInfo& b) noexcept
	{
		return a.width() == b.width() && a.height() == b.height() && a.stride() == b.stride() && a.pixelFormat() == b.pixelFormat() && a.axes() == b.axes();
	}

	inline bool operator==(const Image& a, const Image& b) noexcept
	{
		const auto& info = a.info();
		if (info != b.info())
			return false;
		for (uint32_t y = 0; y < info.height(); ++y)
		{
			const auto aRow = static_cast<const uint8_t*>(a.data()) + y * info.stride();
			const auto bRow = static_cast<const uint8_t*>(b.data()) + y * info.stride();
			if (std::memcmp(aRow, bRow, info.width() * pixelSize(info.pixelFormat())))
				return false;
		}
		return true;
	}
}
