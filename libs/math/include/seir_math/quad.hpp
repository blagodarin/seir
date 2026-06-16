// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_math/vec.hpp>

namespace seir
{
	class Quad
	{
	public:
		Vec2 _a; // Top left.
		Vec2 _b; // Top right.
		Vec2 _c; // Bottom left.
		Vec2 _d; // Bottom right.

		Quad() noexcept = default;
		constexpr Quad(const Vec2& a, const Vec2& b, const Vec2& c, const Vec2& d) noexcept
			: _a{ a }, _b{ b }, _c{ c }, _d{ d } {}
	};
}
