// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_base/utf8.hpp>

#include <string>

#include <doctest/doctest.h>

TEST_CASE("isUtf8Continuation")
{
	unsigned byte = 0;
	for (; byte < 0x80; ++byte)
		CHECK(!seir::isUtf8Continuation(static_cast<char>(byte)));
	for (; byte < 0xc0; ++byte)
		CHECK(seir::isUtf8Continuation(static_cast<char>(byte)));
	for (; byte <= 0xff; ++byte)
		CHECK(!seir::isUtf8Continuation(static_cast<char>(byte)));
}

TEST_CASE("readUtf8")
{
	const auto check = [](const std::string& string, char32_t expectedCodepoint, size_t expectedAdvance) {
		const std::string garbage{ '\xff' };
		auto offset = garbage.size();
		const auto codepoint = seir::readUtf8(garbage + string, offset);
		CHECK(static_cast<uint32_t>(codepoint) == static_cast<uint32_t>(expectedCodepoint));
		CHECK(offset - garbage.size() == expectedAdvance);
	};
	SUBCASE("empty")
	{
		check("", 0, 0);
	}
	SUBCASE("1 byte")
	{
		check("\x01", 0x01, 1);
		check("\x7f", 0x7f, 1);
		check("\x7f\xbf", 0x7f, 1);
	}
	SUBCASE("2 bytes")
	{
		check("\xc2", 0, 1);
		check("\xc2\x80", 0x80, 2);
		check("\xdf\xbf", 0x7ff, 2);
		check("\xdf\xbf\xbf", 0x7ff, 2);
	}
	SUBCASE("3 bytes")
	{
		check("\xe0", 0, 1);
		check("\xe0\xa0", 0, 2);
		check("\xe0\xa0\x80", 0x800, 3);
		check("\xef\xbf\xbf", 0xffff, 3);
		check("\xef\xbf\xbf\xbf", 0xffff, 3);
	}
	SUBCASE("4 bytes")
	{
		check("\xf0", 0, 1);
		check("\xf0\x90", 0, 2);
		check("\xf0\x90\x80", 0, 3);
		check("\xf0\x90\x80\x80", 0x10000, 4);
		check("\xf4\x8f\xbf\xbf", 0x10ffff, 4);
		check("\xf4\x8f\xbf\xbf\xbf", 0x10ffff, 4);
	}
	SUBCASE("invalid")
	{
		// Reading invalid UTF-8 produce wrong codepoints while consuming valid bytes.
		// This is not considered a problem for now.
		check("\x80\x3f\x3f\x3f\x3f", 0x3f, 2);
		check("\x9f\x3f\x3f\x3f\x3f", 0x7ff, 2);
		check("\xa0\x3f\x3f\x3f\x3f", 0xfff, 3);
		check("\xaf\x3f\x3f\x3f\x3f", 0xffff, 3);
		check("\xb0\x3f\x3f\x3f\x3f", 0x3ffff, 4);
		check("\xbf\x3f\x3f\x3f\x3f", 0x1fffff, 4);
	}
}

TEST_CASE("writeUtf8")
{
	const auto writeUtf8 = [](char32_t codepoint) {
		std::array<char, 4> buffer;
		const auto size = seir::writeUtf8(buffer, codepoint);
		return size <= buffer.size() ? std::string{ buffer.data(), size } : std::string{};
	};
	CHECK(writeUtf8(0x0) == std::string{ '\0' });
	CHECK(writeUtf8(0x1) == "\x01");
	CHECK(writeUtf8(0x7f) == "\x7f");
	CHECK(writeUtf8(0x80) == "\xc2\x80");
	CHECK(writeUtf8(0x7ff) == "\xdf\xbf");
	CHECK(writeUtf8(0x800) == "\xe0\xa0\x80");
	CHECK(writeUtf8(0xffff) == "\xef\xbf\xbf");
	CHECK(writeUtf8(0x10000) == "\xf0\x90\x80\x80");
	CHECK(writeUtf8(0x10ffff) == "\xf4\x8f\xbf\xbf");
	CHECK(writeUtf8(0x110000) == "");
	CHECK(writeUtf8(0xffffffff) == "");
}
