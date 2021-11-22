// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <concepts>
#include <cstdint>
#include <type_traits>

namespace seir
{
	// Clamps a signed integer value to a 8-bit unsigned value.
	[[nodiscard]] constexpr uint8_t clampToU8(std::signed_integral auto x) noexcept
	{
		return static_cast<uint8_t>(static_cast<std::make_unsigned_t<decltype(x)>>(x) > 255 ? ~x >> (sizeof x * 8 - 1) : x);
	}

	// Returns true if the value is a power of two.
	[[nodiscard]] constexpr bool isPowerOf2(std::integral auto x) noexcept
	{
		return !(x & (x - 1)) && x > 0;
	}

	// Returns the least power of two not less than the specified positive value.
	template <typename T>
	[[nodiscard]] constexpr std::enable_if_t<std::is_integral_v<T> && sizeof(T) <= sizeof(uint64_t), T> nextPowerOf2(T x) noexcept
	{
		--x;
		x |= x >> 1;
		x |= x >> 2;
		x |= x >> 4;
		if constexpr (sizeof x > sizeof(int8_t))
			x |= x >> 8;
		if constexpr (sizeof x > sizeof(int16_t))
			x |= x >> 16;
		if constexpr (sizeof x > sizeof(int32_t))
			x |= static_cast<std::conditional_t<std::is_signed_v<T>, int64_t, uint64_t>>(x) >> 32;
		return x + 1;
	}

	[[nodiscard]] constexpr auto powerOf2Alignment(std::integral auto x) noexcept
	{
		return static_cast<decltype(x)>(((x ^ (x - 1)) + 1) >> 1);
	}

	// Returns true if both values have the same sign.
	[[nodiscard]] constexpr bool sameSign(std::signed_integral auto x, std::signed_integral auto y) noexcept
	{
		return (x ^ y) >= 0;
	}

	template <typename T>
	[[nodiscard]] constexpr auto toUnderlying(T value) noexcept
	{
		return static_cast<std::underlying_type_t<T>>(value);
	}

	[[nodiscard]] constexpr auto toUnsigned(std::signed_integral auto value) noexcept
	{
		return static_cast<std::make_unsigned_t<decltype(value)>>(value);
	}
}
