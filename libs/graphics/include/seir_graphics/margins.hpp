// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

namespace seir
{
	class Margins
	{
	public:
		int _top = 0;
		int _right = 0;
		int _bottom = 0;
		int _left = 0;

		constexpr Margins() noexcept = default;
		constexpr explicit Margins(int all) noexcept
			: _top{ all }, _right{ all }, _bottom{ all }, _left{ all } {}
		constexpr Margins(int topBottom, int leftRight) noexcept
			: _top{ topBottom }, _right{ leftRight }, _bottom{ topBottom }, _left{ leftRight } {}
		constexpr Margins(int top, int leftRight, int bottom) noexcept
			: _top{ top }, _right{ leftRight }, _bottom{ bottom }, _left{ leftRight } {}
		constexpr Margins(int top, int right, int bottom, int left) noexcept
			: _top{ top }, _right{ right }, _bottom{ bottom }, _left{ left } {}
	};
}
