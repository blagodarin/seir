// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_base/string_utils.hpp>

#include <vector>

#include <ostream>
#include <doctest/doctest.h>

TEST_CASE("matchWildcard")
{
	// To prevent compile time evaluation.
	const auto matchWildcard = [](const std::string& text, const std::string& wildcard) {
		return seir::matchWildcard(text, wildcard);
	};
	SUBCASE("a")
	{
		CHECK(matchWildcard("", ""));
		CHECK(matchWildcard("abc", "abc"));
		CHECK(!matchWildcard("", "abc"));
		CHECK(!matchWildcard("abc", ""));
		CHECK(!matchWildcard("abc", "abcd"));
		CHECK(!matchWildcard("abc", "abd"));
		CHECK(!matchWildcard("abcd", "abc"));
	}
	SUBCASE("?")
	{
		CHECK(matchWildcard("a", "?"));
		CHECK(matchWildcard("abc", "ab?"));
		CHECK(matchWildcard("abc", "?bc"));
		CHECK(matchWildcard("abc", "???"));
		CHECK(!matchWildcard("", "?"));
		CHECK(!matchWildcard("a", "??"));
		CHECK(!matchWildcard("abc", "?"));
		CHECK(!matchWildcard("abc", "??"));
		CHECK(!matchWildcard("abc", "abc?"));
		CHECK(!matchWildcard("abc", "?abc"));
		CHECK(!matchWildcard("abc", "????"));
	}
	SUBCASE("*")
	{
		CHECK(matchWildcard("", "*"));
		CHECK(matchWildcard("", "**"));
		CHECK(matchWildcard("abc", "*"));
		CHECK(matchWildcard("abc", "**"));
		CHECK(matchWildcard("abc", "a*"));
		CHECK(matchWildcard("abc", "*b*"));
		CHECK(matchWildcard("abc", "*c"));
		CHECK(matchWildcard("abc", "abc*"));
		CHECK(matchWildcard("abc", "*abc"));
		CHECK(!matchWildcard("abc", "bc*"));
		CHECK(!matchWildcard("abc", "*ab"));
	}
	SUBCASE("abc*def*fgh")
	{
		using namespace std::string_literals;
		CHECK(matchWildcard("abc"s + "def" + "fgh", "abc*def*fgh"));
		CHECK(matchWildcard("abc"s + "xyz" + "def" + "xyz" + "fgh", "abc*def*fgh"));
		CHECK(matchWildcard("abc"s + "de" + "def" + "fgh", "abc*def*fgh"));
		CHECK(matchWildcard("abc"s + "def" + "def" + "fgh", "abc*def*fgh"));
		CHECK(matchWildcard("abc"s + "def" + "fgh" + "fgh", "abc*def*fgh"));
		CHECK(!matchWildcard("abc"s + "de" + "fgh", "abc*def*fgh"));
		CHECK(!matchWildcard("abc"s + "def" + "fgh" + "xyz", "abc*def*fgh"));
	}
}

TEST_CASE("normalizeWhitespace")
{
	const auto check = [](const std::string& withoutSpace, const std::string& withSpace, const std::vector<std::string_view>& strings) {
		for (const auto string : strings)
		{
			{
				std::string stripped{ string };
				seir::normalizeWhitespace(stripped, seir::TrailingSpace::Remove);
				CHECK(stripped == withoutSpace);
			}
			{
				std::string stripped{ string };
				seir::normalizeWhitespace(stripped, seir::TrailingSpace::Keep);
				CHECK(stripped == withSpace);
			}
		}
	};
	check("", "", { "", " ", "   " });
	check("a", "a", { "a", " a" });
	check("b", "b ", { "b ", " b " });
	check("c d e", "c d e", { "c d e", " c d e" });
	check("f g h", "f g h ", { "f g h ", " f g h " });
	check("ijk", "ijk", { "ijk", "   ijk" });
	check("lmn", "lmn ", { "lmn   ", "   lmn   " });
	check("opq rst", "opq rst", { "opq   rst", "   opq   rst" });
	check("uvw xyz", "uvw xyz ", { "uvw   xyz   ", "   uvw   xyz   " });
}
