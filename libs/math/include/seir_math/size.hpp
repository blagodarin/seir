// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

namespace seir
{
	class Size2D
	{
	public:
		float width = 0;
		float height = 0;

		constexpr Size2D() noexcept = default;
		constexpr Size2D(float w, float h) noexcept
			: width{ w }, height{ h } {}
	};

	[[nodiscard]] constexpr Size2D operator*(const Size2D&, float) noexcept;

	[[nodiscard]] constexpr Size2D operator/(const Size2D&, float) noexcept;
}

constexpr seir::Size2D seir::operator*(const Size2D& a, float b) noexcept
{
	return { a.width * b, a.height * b };
}

constexpr seir::Size2D seir::operator/(const Size2D& a, float b) noexcept
{
	return { a.width / b, a.height / b };
}
