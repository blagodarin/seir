// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_base/base64.hpp>

#include <vector>
#include <string_view>

#include <ostream>
#include <doctest/doctest.h>

TEST_CASE("base64EncodedSize")
{
	SUBCASE("0 blocks")
	{
		CHECK(seir::base64EncodedSize(0) == 0);
	}
	SUBCASE("1 block")
	{
		CHECK(seir::base64EncodedSize(1) == 2);
		CHECK(seir::base64EncodedSize(2) == 3);
		CHECK(seir::base64EncodedSize(3) == 4);
	}
	SUBCASE("2 blocks")
	{
		CHECK(seir::base64EncodedSize(4) == 6);
		CHECK(seir::base64EncodedSize(5) == 7);
		CHECK(seir::base64EncodedSize(6) == 8);
	}
	SUBCASE("3 blocks")
	{
		CHECK(seir::base64EncodedSize(7) == 10);
	}
}

TEST_CASE("encodeBase64Url")
{
	const auto check = [](const std::vector<uint8_t>& input, const std::string_view expected) {
		std::vector<char> output(seir::base64EncodedSize(input.size()), '.');
		REQUIRE(seir::encodeBase64Url(output, std::as_bytes(std::span{ input })));
		CHECK(std::string_view(output.begin(), output.end()) == expected);
	};
	SUBCASE("empty input")
	{
		check({}, "");
	}
	SUBCASE("00 bytes")
	{
		check({ 0x00 }, "AA");
		check({ 0x00, 0x00 }, "AAA");
		check({ 0x00, 0x00, 0x00 }, "AAAA");
		check({ 0x00, 0x00, 0x00, 0x00 }, "AAAAAA");
	}
	SUBCASE("FF bytes")
	{
		check({ 0xFF }, "_w");
		check({ 0xFF, 0xFF }, "__8");
		check({ 0xFF, 0xFF, 0xFF }, "____");
		check({ 0xFF, 0xFF, 0xFF, 0xFF }, "_____w");
	}
	SUBCASE("different bytes")
	{
		check({ 0x22 }, "Ig");
		check({ 0x22, 0x44 }, "IkQ");
		check({ 0x22, 0x44, 0x66 }, "IkRm");
		check({ 0x22, 0x44, 0x66, 0x88 }, "IkRmiA");
	}
	SUBCASE("powers of 64 and 256")
	{
		check({ 0x00, 0x00, 0x01 }, "AAAB"); // 1
		check({ 0x00, 0x00, 0x40 }, "AABA"); // 64
		check({ 0x00, 0x01, 0x00 }, "AAEA"); // 256
		check({ 0x00, 0x10, 0x00 }, "ABAA"); // 64 * 64
		check({ 0x01, 0x00, 0x00 }, "AQAA"); // 256 * 256
		check({ 0x04, 0x00, 0x00 }, "BAAA"); // 64 * 64 * 64
	}
	SUBCASE("alphabet")
	{
		check(
			{
				0x00, 0x10, 0x83, 0x10, 0x51, 0x87, //
				0x20, 0x92, 0x8B, 0x30, 0xD3, 0x8F, //
				0x41, 0x14, 0x93, 0x51, 0x55, 0x97, //
				0x61, 0x96, 0x9B, 0x71, 0xD7, 0x9F, //
				0x82, 0x18, 0xA3, 0x92, 0x59, 0xA7, //
				0xA2, 0x9A, 0xAB, 0xB2, 0xDB, 0xAF, //
				0xC3, 0x1C, 0xB3, 0xD3, 0x5D, 0xB7, //
				0xE3, 0x9E, 0xBB, 0xF3, 0xDF, 0xBF  //
			},
			"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_");
	}
	SUBCASE("bad output size")
	{
		const std::vector<uint8_t> input{ 0x00, 0x01, 0x02, 0x03 };
		std::vector<char> output(seir::base64EncodedSize(input.size()) - 1, '.');
		CHECK_FALSE(seir::encodeBase64Url(output, std::as_bytes(std::span{ input })));
	}
}

TEST_CASE("base64DecodedSize")
{
	SUBCASE("0 blocks")
	{
		CHECK(seir::base64DecodedSize(0) == 0);
	}
	SUBCASE("1 block")
	{
		CHECK(seir::base64DecodedSize(2) == 1);
		CHECK(seir::base64DecodedSize(3) == 2);
		CHECK(seir::base64DecodedSize(4) == 3);
	}
	SUBCASE("2 blocks")
	{
		CHECK(seir::base64DecodedSize(6) == 4);
		CHECK(seir::base64DecodedSize(7) == 5);
		CHECK(seir::base64DecodedSize(8) == 6);
	}
	SUBCASE("3 blocks")
	{
		CHECK(seir::base64DecodedSize(10) == 7);
	}
}

TEST_CASE("decodeBase64")
{
	SUBCASE("valid")
	{
		const auto check = [](const std::string_view input, const std::vector<uint8_t>& expected) {
			std::vector<uint8_t> output(seir::base64DecodedSize(input.size()));
			REQUIRE(seir::decodeBase64Url(std::as_writable_bytes(std::span{ output }), input));
			CHECK(output == expected);
		};
		SUBCASE("empty input")
		{
			check("", {});
		}
		SUBCASE("00 bytes")
		{
			check("AA", { 0x00 });
			check("AAA", { 0x00, 0x00 });
			check("AAAA", { 0x00, 0x00, 0x00 });
			check("AAAAAA", { 0x00, 0x00, 0x00, 0x00 });
		}
		SUBCASE("FF bytes")
		{
			check("_w", { 0xFF });
			check("__8", { 0xFF, 0xFF });
			check("____", { 0xFF, 0xFF, 0xFF });
			check("_____w", { 0xFF, 0xFF, 0xFF, 0xFF });
		}
		SUBCASE("different bytes")
		{
			check("Ig", { 0x22 });
			check("IkQ", { 0x22, 0x44 });
			check("IkRm", { 0x22, 0x44, 0x66 });
			check("IkRmiA", { 0x22, 0x44, 0x66, 0x88 });
		}
		SUBCASE("powers of 64 and 256")
		{
			check("AAAB", { 0x00, 0x00, 0x01 }); // 1
			check("AABA", { 0x00, 0x00, 0x40 }); // 64
			check("AAEA", { 0x00, 0x01, 0x00 }); // 256
			check("ABAA", { 0x00, 0x10, 0x00 }); // 64 * 64
			check("AQAA", { 0x01, 0x00, 0x00 }); // 256 * 256
			check("BAAA", { 0x04, 0x00, 0x00 }); // 64 * 64 * 64
		}
		SUBCASE("alphabet")
		{
			check("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_",
				{
					0x00, 0x10, 0x83, 0x10, 0x51, 0x87, //
					0x20, 0x92, 0x8B, 0x30, 0xD3, 0x8F, //
					0x41, 0x14, 0x93, 0x51, 0x55, 0x97, //
					0x61, 0x96, 0x9B, 0x71, 0xD7, 0x9F, //
					0x82, 0x18, 0xA3, 0x92, 0x59, 0xA7, //
					0xA2, 0x9A, 0xAB, 0xB2, 0xDB, 0xAF, //
					0xC3, 0x1C, 0xB3, 0xD3, 0x5D, 0xB7, //
					0xE3, 0x9E, 0xBB, 0xF3, 0xDF, 0xBF  //
				});
		}
	}
	SUBCASE("invalid")
	{
		const auto check = [](const std::string_view input, bool underflow = false) {
			std::vector<uint8_t> output(seir::base64DecodedSize(input.size()) - static_cast<size_t>(underflow));
			CHECK_FALSE(seir::decodeBase64Url(std::as_writable_bytes(std::span{ output }), input));
		};
		SUBCASE("bad input size")
		{
			check("A");
			check("AAAAA");
		}
		SUBCASE("bad input data")
		{
			check("AB+/");
			check("ABCD+/");
		}
		SUBCASE("bad output size")
		{
			check("AAAA", true);
			check("AAAAAA", true);
			check("AAAAAAA", true);
		}
	}
}
