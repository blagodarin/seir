// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_math/line.hpp>

#include <seir_math/plane.hpp>

#include <doctest/doctest.h>

TEST_CASE("Line3::intersection")
{
	const seir::Plane plane{ { 1, 1.5f, 3 }, { 2, 3, 6 } };
	SUBCASE("forward")
	{
		const seir::Line3 line{ { 0, 0, 0 }, { 4, 6, 12 } };
		const auto intersection = line.intersection(plane);
		REQUIRE(intersection);
		CHECK(*intersection == seir::Vec3(2, 3, 6));
	}
	SUBCASE("backward")
	{
		const seir::Line3 line{ { 4, 6, 12 }, { 0, 0, 0 } };
		const auto intersection = line.intersection(plane);
		REQUIRE(intersection);
		CHECK(*intersection == seir::Vec3(2, 3, 6));
	}
	SUBCASE("none")
	{
		const seir::Line3 line{ { 0, 0, 0 }, { 2, 3, 5 } };
		const auto intersection = line.intersection(plane);
		CHECK_FALSE(intersection);
	}
}
