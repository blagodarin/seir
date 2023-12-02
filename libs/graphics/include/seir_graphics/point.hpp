// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

namespace seir
{
	class Point
	{
	public:
		int _x = 0;
		int _y = 0;

		constexpr Point() noexcept = default;
		constexpr Point(int x, int y) noexcept
			: _x{ x }, _y{ y } {}
	};

	[[nodiscard]] constexpr bool operator==(const Point& a, const Point& b) noexcept { return a._x == b._x && a._y == b._y; }
}
