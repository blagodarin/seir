// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_graphics/margins.hpp>

namespace seir
{
	class MarginsF
	{
	public:
		float _top = 0;
		float _right = 0;
		float _bottom = 0;
		float _left = 0;

		constexpr MarginsF() noexcept = default;
		constexpr explicit MarginsF(float all) noexcept
			: _top{ all }, _right{ all }, _bottom{ all }, _left{ all } {}
		constexpr MarginsF(float topBottom, float leftRight) noexcept
			: _top{ topBottom }, _right{ leftRight }, _bottom{ topBottom }, _left{ leftRight } {}
		constexpr MarginsF(float top, float leftRight, float bottom) noexcept
			: _top{ top }, _right{ leftRight }, _bottom{ bottom }, _left{ leftRight } {}
		constexpr MarginsF(float top, float right, float bottom, float left) noexcept
			: _top{ top }, _right{ right }, _bottom{ bottom }, _left{ left } {}
		constexpr explicit MarginsF(const Margins& other) noexcept
			: MarginsF{ static_cast<float>(other._top), static_cast<float>(other._right), static_cast<float>(other._bottom), static_cast<float>(other._left) } {}
	};
}
