// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

namespace seir
{
	class SizeF
	{
	public:
		float _width = 0;
		float _height = 0;

		constexpr SizeF() noexcept = default;
		constexpr SizeF(float width, float height) noexcept
			: _width{ width }, _height{ height } {}
	};

	[[nodiscard]] constexpr SizeF operator*(const SizeF&, float) noexcept;

	[[nodiscard]] constexpr SizeF operator/(const SizeF&, float) noexcept;
}

constexpr seir::SizeF seir::operator*(const SizeF& a, float b) noexcept
{
	return { a._width * b, a._height * b };
}

constexpr seir::SizeF seir::operator/(const SizeF& a, float b) noexcept
{
	return { a._width / b, a._height / b };
}
