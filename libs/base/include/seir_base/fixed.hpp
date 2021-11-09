// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cmath>
#include <cstdint>

namespace seir
{
	// Fixed-point storage.
	template <typename T, unsigned kFractionBits>
	class Fixed
	{
	public:
		constexpr Fixed() noexcept = default;
		constexpr explicit Fixed(float value) noexcept
			: _value{ static_cast<T>(value * kOne) } {}

		[[nodiscard]] constexpr explicit operator float() const noexcept { return static_cast<float>(_value) / kOne; }

		[[nodiscard]] constexpr T store() const noexcept { return _value; }

		[[nodiscard]] static Fixed ceil(float value) noexcept { return Fixed{ static_cast<T>(std::ceil(value * kOne)) }; }
		[[nodiscard]] static constexpr Fixed load(T value) noexcept { return Fixed{ value }; }

	private:
		T _value = 0;
		static constexpr T kOne = 1 << kFractionBits;
		constexpr explicit Fixed(T value) noexcept
			: _value{ value } {}
	};
}
