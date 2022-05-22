// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_base/int_utils.hpp>

#include <array>
#include <cstddef>
#include <limits>

#include <doctest/doctest.h>

TEST_CASE("clampToU8")
{
	using seir::clampToU8;
	CHECK(clampToU8(std::numeric_limits<int>::min()) == 0);
	CHECK(clampToU8(-1) == 0);
	CHECK(clampToU8(0) == 0);
	CHECK(clampToU8(1) == 1);
	CHECK(clampToU8(254) == 254);
	CHECK(clampToU8(255) == 255);
	CHECK(clampToU8(256) == 255);
	CHECK(clampToU8(std::numeric_limits<int>::max()) == 255);
}

TEST_CASE("isPowerOf2")
{
	static const std::array<int, std::numeric_limits<int8_t>::max() + 1> table{
		0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};
	using seir::isPowerOf2;
	for (int8_t i = std::numeric_limits<int8_t>::min(); i < 0; ++i)
	{
		INFO("i = " << i);
		CHECK(!isPowerOf2(i));
	}
	for (uint8_t i = 0; i <= std::numeric_limits<int8_t>::max(); ++i)
	{
		INFO("i = " << i);
		CHECK(int{ isPowerOf2(i) } == table[i]);
	}
}

TEST_CASE("nextPowerOf2")
{
	static const std::array<uint8_t, std::numeric_limits<int8_t>::max() + 1> table{
		0, 1, 2, 4, 4, 8, 8, 8, 8, 16, 16, 16, 16, 16, 16, 16,
		16, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
		32, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128
	};
	using seir::nextPowerOf2;
	for (std::size_t i = 1; i <= static_cast<size_t>(std::numeric_limits<int8_t>::max()); ++i)
	{
		INFO("i = " << i);
		CHECK(nextPowerOf2(i) == table[i]);
	}
	constexpr auto maxPowerOf2 = static_cast<uintmax_t>(std::numeric_limits<intmax_t>::max()) + 1;
	CHECK(nextPowerOf2(maxPowerOf2 - 1) == maxPowerOf2);
	CHECK(nextPowerOf2(maxPowerOf2) == maxPowerOf2);
	CHECK(nextPowerOf2(maxPowerOf2 + 1) != maxPowerOf2);
}

TEST_CASE("powerOf2Alignment")
{
	static const std::array<uint8_t, std::numeric_limits<int8_t>::max() + 1> table{
		0, 1, 2, 1, 4, 1, 2, 1, 8, 1, 2, 1, 4, 1, 2, 1,
		16, 1, 2, 1, 4, 1, 2, 1, 8, 1, 2, 1, 4, 1, 2, 1,
		32, 1, 2, 1, 4, 1, 2, 1, 8, 1, 2, 1, 4, 1, 2, 1,
		16, 1, 2, 1, 4, 1, 2, 1, 8, 1, 2, 1, 4, 1, 2, 1,
		64, 1, 2, 1, 4, 1, 2, 1, 8, 1, 2, 1, 4, 1, 2, 1,
		16, 1, 2, 1, 4, 1, 2, 1, 8, 1, 2, 1, 4, 1, 2, 1,
		32, 1, 2, 1, 4, 1, 2, 1, 8, 1, 2, 1, 4, 1, 2, 1,
		16, 1, 2, 1, 4, 1, 2, 1, 8, 1, 2, 1, 4, 1, 2, 1
	};
	using seir::powerOf2Alignment;
	for (int i = 0; i <= std::numeric_limits<int8_t>::max(); ++i)
	{
		INFO("i = " << i);
		CHECK(powerOf2Alignment(i) == table[static_cast<size_t>(i)]);
		CHECK(powerOf2Alignment(-i) == table[static_cast<size_t>(i)]);
	}
}

TEST_CASE("sameSign")
{
	using seir::sameSign;
	CHECK(sameSign<intmax_t>(0, 0));
	CHECK(sameSign<intmax_t>(-1, -1));
	CHECK(!sameSign<intmax_t>(0, -1));
	CHECK(!sameSign<intmax_t>(-1, 0));
	CHECK(sameSign(std::numeric_limits<intmax_t>::max(), std::numeric_limits<intmax_t>::max()));
	CHECK(sameSign(std::numeric_limits<intmax_t>::min(), std::numeric_limits<intmax_t>::min()));
	CHECK(!sameSign(std::numeric_limits<intmax_t>::max(), std::numeric_limits<intmax_t>::min()));
	CHECK(!sameSign(std::numeric_limits<intmax_t>::min(), std::numeric_limits<intmax_t>::max()));
}
