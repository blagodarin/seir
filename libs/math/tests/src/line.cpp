// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_math/line.hpp>

#include <seir_math/plane.hpp>

#include <doctest/doctest.h>

TEST_CASE("Line3::intersection")
{
	const seir::Plane plane{ { 1, 1.5f, 3 }, { 2, 3, 6 } };
	SUBCASE("forward hit")
	{
		const seir::Line3 line{ { 0, 0, 0 }, { 4, 6, 12 } };
		const auto intersection = line.intersection(plane);
		REQUIRE(intersection);
		CHECK(*intersection == seir::Vec3(2, 3, 6));
	}
	SUBCASE("forward miss")
	{
		const seir::Line3 line{ { 0, 0, 0 }, { 2, 3, 5 } };
		CHECK_FALSE(line.intersection(plane));
	}
	SUBCASE("backward hit")
	{
		const seir::Line3 line{ { 4, 6, 12 }, { 0, 0, 0 } };
		const auto intersection = line.intersection(plane);
		REQUIRE(intersection);
		CHECK(*intersection == seir::Vec3(2, 3, 6));
	}
	SUBCASE("backward miss")
	{
		const seir::Line3 line{ { 4, 6, 12 }, { 2, 3, 7 } };
		CHECK_FALSE(line.intersection(plane));
	}
	SUBCASE("parallel")
	{
		const seir::Line3 line{ { 2, 3, 6 }, { 5, 7, 3 } };                         // Both points are on the plane.
		REQUIRE(plane.distanceTo(line.first()) == plane.distanceTo(line.second())); // Ensure the line is parallel to the plane.
		CHECK_FALSE(line.intersection(plane));
	}
}
