// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_graphics/color.hpp>

#include <doctest/doctest.h>

using seir::Rgba32;

TEST_CASE("Rgba32::Rgba32()")
{
	Rgba32 c;
	CHECK(c._r == 0);
	CHECK(c._g == 0);
	CHECK(c._b == 0);
	CHECK(c._a == 0);
}

TEST_CASE("Rgba32::Rgba32(R, G, B)")
{
	Rgba32 c{ 1, 2, 3 };
	CHECK(c._r == 1);
	CHECK(c._g == 2);
	CHECK(c._b == 3);
	CHECK(c._a == 255);
}

TEST_CASE("Rgba32::Rgba32(R, G, B, A)")
{
	Rgba32 c{ 1, 2, 3, 4 };
	CHECK(c._r == 1);
	CHECK(c._g == 2);
	CHECK(c._b == 3);
	CHECK(c._a == 4);
}
