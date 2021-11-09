// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_base/fixed.hpp>

#include <doctest/doctest.h>

namespace
{
	using Fixed = seir::Fixed<unsigned, 4>;
}

TEST_CASE("Fixed::Fixed()")
{
	const Fixed value;
	CHECK(value.store() == 0);
	CHECK(static_cast<float>(value) == 0.f);
}

TEST_CASE("Fixed::Fixed(float)")
{
	const Fixed value{ 1.f };
	CHECK(value.store() == 1 << 4);
	CHECK(static_cast<float>(value) == 1.f);
}

TEST_CASE("Fixed::load(T)")
{
	const auto value = Fixed::load(1);
	CHECK(value.store() == 1);
	CHECK(static_cast<float>(value) == std::exp2(-4.f));
}

TEST_CASE("Fixed::ceil(float)")
{
	SUBCASE("1")
	{
		const auto value = Fixed::ceil(std::exp2(-5.f));
		CHECK(value.store() == 1);
		CHECK(static_cast<float>(value) == std::exp2(-4.f));
	}
	SUBCASE("3")
	{
		const auto value = Fixed::ceil(5 * std::exp2(-5.f));
		CHECK(value.store() == 3);
		CHECK(static_cast<float>(value) == 1.5f * std::exp2(-3.f));
	}
}
