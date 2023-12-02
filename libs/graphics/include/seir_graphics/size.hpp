// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

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
	};

	[[nodiscard]] constexpr bool operator==(const Size& a, const Size& b) noexcept { return a._width == b._width && a._height == b._height; }
}
