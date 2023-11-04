// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <array>
#include <cstdint>
#include <string_view>

namespace seir
{
	[[nodiscard]] constexpr bool isUtf8Continuation(char c) noexcept
	{
		return (static_cast<unsigned char>(c) & 0b1100'0000u) == 0b1000'0000u;
	}

	[[nodiscard]] constexpr char32_t readUtf8(std::string_view text, size_t& offset) noexcept
	{
		if (offset >= text.size())
			return 0;
		const auto part1 = static_cast<uint8_t>(text[offset++]);
		if (!(part1 & 0b1000'0000))
			return part1;
		if (offset == text.size())
			return 0;
		const auto part2 = char32_t{ static_cast<uint8_t>(text[offset++]) & 0b0011'1111u };
		if (!(part1 & 0b0010'0000))
			return ((part1 & 0b0001'1111u) << 6) + part2;
		if (offset == text.size())
			return 0;
		const auto part3 = char32_t{ static_cast<uint8_t>(text[offset++]) & 0b0011'1111u };
		if (!(part1 & 0b0001'0000))
			return ((part1 & 0b0000'1111u) << 12) + (part2 << 6) + part3;
		if (offset == text.size())
			return 0;
		const auto part4 = char32_t{ static_cast<uint8_t>(text[offset++]) & 0b0011'1111u };
		return ((part1 & 0b0000'0111u) << 18) + (part2 << 12) + (part3 << 6) + part4;
	}

	[[nodiscard]] constexpr size_t writeUtf8(std::array<char, 4>& buffer, char32_t codepoint) noexcept
	{
		if (codepoint <= 0x7f)
		{
			buffer[0] = static_cast<char>(codepoint);
			return 1;
		}
		if (codepoint <= 0x7ff)
		{
			buffer[0] = static_cast<char>(0b1100'0000u | (0b0001'1111u & (codepoint >> 6)));
			buffer[1] = static_cast<char>(0b1000'0000u | (0b0011'1111u & codepoint));
			return 2;
		}
		if (codepoint <= 0xffff)
		{
			buffer[0] = static_cast<char>(0b1110'0000u | (0b0000'1111u & (codepoint >> 12)));
			buffer[1] = static_cast<char>(0b1000'0000u | (0b0011'1111u & (codepoint >> 6)));
			buffer[2] = static_cast<char>(0b1000'0000u | (0b0011'1111u & codepoint));
			return 3;
		}
		if (codepoint <= 0x10ffff)
		{
			buffer[0] = static_cast<char>(0b1111'0000u | (0b0000'0111u & (codepoint >> 18)));
			buffer[1] = static_cast<char>(0b1000'0000u | (0b0011'1111u & (codepoint >> 12)));
			buffer[2] = static_cast<char>(0b1000'0000u | (0b0011'1111u & (codepoint >> 6)));
			buffer[3] = static_cast<char>(0b1000'0000u | (0b0011'1111u & codepoint));
			return 4;
		}
		return 0;
	}
}
