// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <bit>
#include <cstdint>

namespace seir
{
	[[nodiscard]] constexpr uint16_t swapBytes(uint16_t x) noexcept { return static_cast<uint16_t>(x >> 8 | x << 8); }
	[[nodiscard]] constexpr uint32_t swapBytes(uint32_t x) noexcept { return x << 24 | (x & 0xff00) << 8 | (x & 0xff0000) >> 8 | x >> 24; }

	template <typename T>
	[[nodiscard]] constexpr T bigEndian(T x) noexcept
	{
		return std::endian::native == std::endian::little ? swapBytes(x) : x;
	}

	template <typename T>
	[[nodiscard]] constexpr T littleEndian(T x) noexcept
	{
		return std::endian::native == std::endian::big ? swapBytes(x) : x;
	}

	[[nodiscard]] constexpr uint16_t makeCC(char c0, char c1) noexcept
	{
		constexpr auto shift = std::endian::native == std::endian::little ? 0 : 8;
		return static_cast<uint16_t>(static_cast<uint8_t>(c0) << shift | static_cast<uint8_t>(c1) << (8 - shift));
	}

	[[nodiscard]] constexpr uint32_t makeCC(char c0, char c1, char c2, char c3) noexcept
	{
		constexpr auto shift = std::endian::native == std::endian::little ? 0 : 16;
		return uint32_t{ makeCC(c0, c1) } << shift | uint32_t{ makeCC(c2, c3) } << (16 - shift);
	}

	[[nodiscard]] constexpr uint64_t makeCC(char c0, char c1, char c2, char c3, char c4, char c5, char c6, char c7) noexcept
	{
		constexpr auto shift = std::endian::native == std::endian::little ? 0 : 32;
		return uint64_t{ makeCC(c0, c1, c2, c3) } << shift | uint64_t{ makeCC(c4, c5, c6, c7) } << (32 - shift);
	}
}
