// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_graphics/point.hpp>

#include <doctest/doctest.h>

using seir::Point;

TEST_CASE("Point::Point()")
{
	Point p;
	CHECK(p._x == 0);
	CHECK(p._y == 0);
}

TEST_CASE("Point::Point(int, int)")
{
	Point p{ 1, 2 };
	CHECK(p._x == 1);
	CHECK(p._y == 2);
}
