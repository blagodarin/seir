// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_base/endian.hpp>

#include <array>
#include <cstring>

#include <doctest/doctest.h>

TEST_CASE("bigEndian(T)")
{
	if constexpr (std::endian::native == std::endian::little)
	{
		CHECK(seir::bigEndian(uint16_t{ 0x8081 }) == 0x8180);
		CHECK(seir::bigEndian(uint32_t{ 0x80818283 }) == 0x83828180);
	}
	if constexpr (std::endian::native == std::endian::big)
	{
		CHECK(seir::bigEndian(uint16_t{ 0x8081 }) == 0x8081);
		CHECK(seir::bigEndian(uint32_t{ 0x80818283 }) == 0x80818283);
	}
}

TEST_CASE("first16(T)")
{
	const std::array<uint8_t, 8> bytes{ 0, 1, 2, 3, 4, 5, 6, 7 };
	CHECK(seir::first16(*reinterpret_cast<const uint32_t*>(bytes.data())) == *reinterpret_cast<const uint16_t*>(bytes.data()));
	CHECK(seir::first16(*reinterpret_cast<const uint64_t*>(bytes.data())) == *reinterpret_cast<const uint16_t*>(bytes.data()));
}

TEST_CASE("littleEndian(T)")
{
	if constexpr (std::endian::native == std::endian::little)
	{
		CHECK(seir::littleEndian(uint16_t{ 0x8081 }) == 0x8081);
		CHECK(seir::littleEndian(uint32_t{ 0x80818283 }) == 0x80818283);
	}
	if constexpr (std::endian::native == std::endian::big)
	{
		CHECK(seir::littleEndian(uint16_t{ 0x8081 }) == 0x8180);
		CHECK(seir::littleEndian(uint32_t{ 0x80818283 }) == 0x83828180);
	}
}

TEST_CASE("makeCC(char, char)")
{
	constexpr auto value = seir::makeCC('\x01', '\xff');
	CHECK(std::memcmp(&value, "\x01\xff", sizeof value) == 0);
}

TEST_CASE("makeCC(char, char, char, char)")
{
	constexpr auto value = seir::makeCC('\x01', '\x80', '\x7f', '\xff');
	CHECK(std::memcmp(&value, "\x01\x80\x7f\xff", sizeof value) == 0);
}

TEST_CASE("makeCC(char, char, char, char, char, char, char, char)")
{
	constexpr auto value = seir::makeCC('\x01', '\x02', '\x7e', '\x7f', '\x80', '\x81', '\xfe', '\xff');
	CHECK(std::memcmp(&value, "\x01\x02\x7e\x7f\x80\x81\xfe\xff", sizeof value) == 0);
}

TEST_CASE("swapBytes(uint16_t)")
{
	CHECK(seir::swapBytes(uint16_t{ 0x8081 }) == 0x8180);
}

TEST_CASE("swapBytes(uint32_t)")
{
	CHECK(seir::swapBytes(uint32_t{ 0x80818283 }) == 0x83828180);
}
