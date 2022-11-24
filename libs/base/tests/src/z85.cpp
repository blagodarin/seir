// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_base/z85.hpp>

#include <array>
#include <string_view>

#include <ostream>
#include <doctest/doctest.h>

TEST_CASE("encodeZ85")
{
	const auto check = [](const std::array<uint8_t, 4>& input, const std::string_view expected) {
		std::array<char, 5> output{};
		seir::encodeZ85(output, std::as_bytes(std::span{ input }));
		CHECK(std::string_view(output.begin(), output.end()) == expected);
	};
	check({ 0x00, 0x00, 0x00, 0x00 }, "00000");
	check({ 0x00, 0x00, 0x00, 0x01 }, "00001");
	check({ 0x00, 0x00, 0x00, 0x55 }, "00010");
	check({ 0x86, 0x4F, 0xD2, 0x6F }, "Hello");
	check({ 0xFF, 0xFF, 0xFF, 0xFF }, "%nSc0");
}

TEST_CASE("decodeZ85")
{
	SUBCASE("valid")
	{
		const auto check = [](const std::string_view input, const std::array<uint8_t, 4>& expected) {
			std::array<uint8_t, 4> output{};
			REQUIRE(input.size() == 5);
			REQUIRE(seir::decodeZ85(std::as_writable_bytes(std::span{ output }), std::span{ input }.subspan<0, 5>()));
			CHECK(output == expected);
		};
		check("00000", { 0x00, 0x00, 0x00, 0x00 }); // 0
		check("00001", { 0x00, 0x00, 0x00, 0x01 }); // 1
		check("00010", { 0x00, 0x00, 0x00, 0x55 }); // 85
		check("Hello", { 0x86, 0x4F, 0xD2, 0x6F }); //
		check("%nSc0", { 0xFF, 0xFF, 0xFF, 0xFF }); // 2^32 - 1
	}
	SUBCASE("invalid")
	{
		const auto check = [](const std::string_view input) {
			std::array<uint8_t, 4> output{};
			REQUIRE(input.size() == 5);
			CHECK_FALSE(seir::decodeZ85(std::as_writable_bytes(std::span{ output }), std::span{ input }.subspan<0, 5>()));
		};
		check("%nSc1"); // 2^32
		check("#####"); // 85^5 - 1 = 2^32 + 142'085'829
	}
}
