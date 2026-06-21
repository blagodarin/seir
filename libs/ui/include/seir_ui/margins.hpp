// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

namespace seir
{
	class Margins
	{
	public:
		float _top = 0;
		float _right = 0;
		float _bottom = 0;
		float _left = 0;

		constexpr Margins() noexcept = default;
		constexpr explicit Margins(float all) noexcept
			: _top{ all }, _right{ all }, _bottom{ all }, _left{ all } {}
		constexpr Margins(float topBottom, float leftRight) noexcept
			: _top{ topBottom }, _right{ leftRight }, _bottom{ topBottom }, _left{ leftRight } {}
		constexpr Margins(float top, float leftRight, float bottom) noexcept
			: _top{ top }, _right{ leftRight }, _bottom{ bottom }, _left{ leftRight } {}
		constexpr Margins(float top, float right, float bottom, float left) noexcept
			: _top{ top }, _right{ right }, _bottom{ bottom }, _left{ left } {}
	};
}
