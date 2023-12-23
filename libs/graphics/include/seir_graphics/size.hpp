// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstddef>

namespace seir
{
	class Size
	{
	public:
		int _width = 0;
		int _height = 0;

		constexpr Size() noexcept = default;
		constexpr Size(int width, int height) noexcept
			: _width{ width }, _height{ height } {}
		constexpr Size(size_t width, size_t height) noexcept
			: _width{ static_cast<int>(width) }, _height{ static_cast<int>(height) } {}
	};

	[[nodiscard]] constexpr bool operator==(const Size& a, const Size& b) noexcept { return a._width == b._width && a._height == b._height; }
}
