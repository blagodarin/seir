// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstdint>
#include <type_traits>

namespace seir
{
	class Rgba32
	{
	public:
		uint8_t _r = 0;
		uint8_t _g = 0;
		uint8_t _b = 0;
		uint8_t _a = 0;

		constexpr Rgba32() noexcept = default;

		template <typename R, typename G, typename B, typename = std::enable_if_t<std::is_integral_v<R> && std::is_integral_v<G> && std::is_integral_v<B>>>
		constexpr explicit Rgba32(R r, G g, B b) noexcept
			: _r{ static_cast<uint8_t>(r) }, _g{ static_cast<uint8_t>(g) }, _b{ static_cast<uint8_t>(b) }, _a{ 255 } {}

		template <typename R, typename G, typename B, typename A, typename = std::enable_if_t<std::is_integral_v<R> && std::is_integral_v<G> && std::is_integral_v<B> && std::is_integral_v<A>>>
		constexpr explicit Rgba32(R r, G g, B b, A a) noexcept
			: _r{ static_cast<uint8_t>(r) }, _g{ static_cast<uint8_t>(g) }, _b{ static_cast<uint8_t>(b) }, _a{ static_cast<uint8_t>(a) } {}

		[[nodiscard]] static constexpr Rgba32 black(uint8_t alpha = 255) noexcept { return Rgba32{ 0, 0, 0, alpha }; }
		[[nodiscard]] static constexpr Rgba32 blue(uint8_t alpha = 255) noexcept { return Rgba32{ 0, 0, 255, alpha }; }
		[[nodiscard]] static constexpr Rgba32 cyan(uint8_t alpha = 255) noexcept { return Rgba32{ 0, 255, 255, alpha }; }
		[[nodiscard]] static constexpr Rgba32 grayscale(uint8_t color, uint8_t alpha = 255) noexcept { return Rgba32{ color, color, color, alpha }; }
		[[nodiscard]] static constexpr Rgba32 green(uint8_t alpha = 255) noexcept { return Rgba32{ 0, 255, 0, alpha }; }
		[[nodiscard]] static constexpr Rgba32 magenta(uint8_t alpha = 255) noexcept { return Rgba32{ 255, 0, 255, alpha }; }
		[[nodiscard]] static constexpr Rgba32 red(uint8_t alpha = 255) noexcept { return Rgba32{ 255, 0, 0, alpha }; }
		[[nodiscard]] static constexpr Rgba32 white(uint8_t alpha = 255) noexcept { return Rgba32{ 255, 255, 255, alpha }; }
		[[nodiscard]] static constexpr Rgba32 yellow(uint8_t alpha = 255) noexcept { return Rgba32{ 255, 255, 0, alpha }; }
	};

	[[nodiscard]] constexpr bool operator==(const Rgba32& a, const Rgba32& b) noexcept { return a._b == b._b && a._g == b._g && a._r == b._r && a._a == b._a; }
}
