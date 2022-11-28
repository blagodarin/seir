// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_base/base85.hpp>

#include <array>
#include <string_view>
#include <vector>

#include <ostream>
#include <doctest/doctest.h>

TEST_CASE("base85EncodedSize")
{
	SUBCASE("0 blocks")
	{
		CHECK(seir::base85EncodedSize(0) == 0);
	}
	SUBCASE("1 block")
	{
		CHECK(seir::base85EncodedSize(1) == 2);
		CHECK(seir::base85EncodedSize(2) == 3);
		CHECK(seir::base85EncodedSize(3) == 4);
		CHECK(seir::base85EncodedSize(4) == 5);
	}
	SUBCASE("2 blocks")
	{
		CHECK(seir::base85EncodedSize(5) == 7);
		CHECK(seir::base85EncodedSize(6) == 8);
		CHECK(seir::base85EncodedSize(7) == 9);
		CHECK(seir::base85EncodedSize(8) == 10);
	}
	SUBCASE("3 blocks")
	{
		CHECK(seir::base85EncodedSize(9) == 12);
	}
}

TEST_CASE("encodeZ85")
{
	const auto check = [](const std::vector<uint8_t>& input, const std::string_view expected) {
		std::vector<char> output(seir::base85EncodedSize(input.size()), '.');
		REQUIRE(seir::encodeZ85(output, std::as_bytes(std::span{ input })));
		CHECK(std::string_view(output.begin(), output.end()) == expected);
	};
	SUBCASE("empty input")
	{
		check({}, "");
	}
	SUBCASE("00 bytes")
	{
		check({ 0x00 }, "00");
		check({ 0x00, 0x00 }, "000");
		check({ 0x00, 0x00, 0x00 }, "0000");
		check({ 0x00, 0x00, 0x00, 0x00 }, "00000");
		check({ 0x00, 0x00, 0x00, 0x00, 0x00 }, "0000000");
	}
	SUBCASE("FF bytes")
	{
		check({ 0xFF }, "@%");
		check({ 0xFF, 0xFF }, "%nK");
		check({ 0xFF, 0xFF, 0xFF }, "%nS9");
		check({ 0xFF, 0xFF, 0xFF, 0xFF }, "%nSc0");
		check({ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }, "%nSc0@%");
	}
	SUBCASE("powers of 85 and 256")
	{
		check({ 0x00, 0x00, 0x00, 0x01 }, "00001"); // 1
		check({ 0x00, 0x00, 0x00, 0x55 }, "00010"); // 85
		check({ 0x00, 0x00, 0x01, 0x00 }, "00031"); // 256
		check({ 0x00, 0x00, 0x1C, 0x39 }, "00100"); // 85 * 85
		check({ 0x00, 0x01, 0x00, 0x00 }, "00961"); // 256 * 256
		check({ 0x00, 0x09, 0x5E, 0xED }, "01000"); // 85 * 85 * 85
		check({ 0x01, 0x00, 0x00, 0x00 }, "0rr91"); // 256 * 256 * 256
		check({ 0x03, 0x1C, 0x84, 0xB1 }, "10000"); // 85 * 85 * 85 * 85
	}
	SUBCASE("bad output size")
	{
		const std::vector<uint8_t> input{ 0x00, 0x01, 0x02, 0x03, 0x04 };
		std::vector<char> output(seir::base85EncodedSize(input.size()) - 1, '_');
		CHECK_FALSE(seir::encodeZ85(output, std::as_bytes(std::span{ input })));
	}
}

TEST_CASE("base85DecodedSize")
{
	SUBCASE("0 blocks")
	{
		CHECK(seir::base85DecodedSize(0) == 0);
	}
	SUBCASE("1 block")
	{
		CHECK(seir::base85DecodedSize(2) == 1);
		CHECK(seir::base85DecodedSize(3) == 2);
		CHECK(seir::base85DecodedSize(4) == 3);
		CHECK(seir::base85DecodedSize(5) == 4);
	}
	SUBCASE("2 blocks")
	{
		CHECK(seir::base85DecodedSize(7) == 5);
		CHECK(seir::base85DecodedSize(8) == 6);
		CHECK(seir::base85DecodedSize(9) == 7);
		CHECK(seir::base85DecodedSize(10) == 8);
	}
	SUBCASE("3 blocks")
	{
		CHECK(seir::base85DecodedSize(12) == 9);
	}
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

TEST_CASE("decodeZ85")
{
	SUBCASE("valid")
	{
		const auto check = [](const std::string_view input, const std::vector<uint8_t>& expected) {
			std::vector<uint8_t> output(seir::base85DecodedSize(input.size()));
			REQUIRE(seir::decodeZ85(std::as_writable_bytes(std::span{ output }), input));
			CHECK(output == expected);
		};
		SUBCASE("empty input")
		{
			check("", {});
		}
		SUBCASE("00 bytes")
		{
			check("00", { 0x00 });
			check("000", { 0x00, 0x00 });
			check("0000", { 0x00, 0x00, 0x00 });
			check("00000", { 0x00, 0x00, 0x00, 0x00 });
			check("0000000", { 0x00, 0x00, 0x00, 0x00, 0x00 });
		}
		SUBCASE("FF bytes")
		{
			check("@%", { 0xFF });
			check("%nK", { 0xFF, 0xFF });
			check("%nS9", { 0xFF, 0xFF, 0xFF });
			check("%nSc0", { 0xFF, 0xFF, 0xFF, 0xFF });
			check("%nSc0@%", { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF });
		}
		SUBCASE("powers of 85 and 256")
		{
			check("00001", { 0x00, 0x00, 0x00, 0x01 }); // 1
			check("00010", { 0x00, 0x00, 0x00, 0x55 }); // 85
			check("00031", { 0x00, 0x00, 0x01, 0x00 }); // 256
			check("00100", { 0x00, 0x00, 0x1C, 0x39 }); // 85 * 85
			check("00961", { 0x00, 0x01, 0x00, 0x00 }); // 256 * 256
			check("01000", { 0x00, 0x09, 0x5E, 0xED }); // 85 * 85 * 85
			check("0rr91", { 0x01, 0x00, 0x00, 0x00 }); // 256 * 256 * 256
			check("10000", { 0x03, 0x1C, 0x84, 0xB1 }); // 85 * 85 * 85 * 85
		}
	}
	SUBCASE("invalid")
	{
		const auto check = [](const std::string_view input, bool underflow = false) {
			std::vector<uint8_t> output(seir::base85DecodedSize(input.size()) - static_cast<size_t>(underflow));
			CHECK_FALSE(seir::decodeZ85(std::as_writable_bytes(std::span{ output }), input));
		};
		SUBCASE("bad input size")
		{
			check("0");
			check("000000");
		}
		SUBCASE("bad input data")
		{
			check("0000_");
			check("00000_");
		}
		SUBCASE("out of range")
		{
			check("%nSc1"); // 2^32
			check("#####"); // 85^5 - 1 = 2^32 + 142'085'829
		}
		SUBCASE("bad output size")
		{
			check("00000", true);
			check("0000000", true);
			check("00000000", true);
			check("000000000", true);
		}
	}
}
