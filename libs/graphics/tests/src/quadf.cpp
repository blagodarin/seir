// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_graphics/quadf.hpp>

#include <doctest/doctest.h>

using seir::QuadF;

TEST_CASE("QuadF::QuadF(Vec2, Vec2, Vec2, Vec2)")
{
	const QuadF q{ { 1, 2 }, { 3, 4 }, { 5, 6 }, { 7, 8 } };
	CHECK(q._a.x == 1.f);
	CHECK(q._a.y == 2.f);
	CHECK(q._b.x == 3.f);
	CHECK(q._b.y == 4.f);
	CHECK(q._c.x == 5.f);
	CHECK(q._c.y == 6.f);
	CHECK(q._d.x == 7.f);
	CHECK(q._d.y == 8.f);
}
