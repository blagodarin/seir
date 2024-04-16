// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_math/plane.hpp>

#include <doctest/doctest.h>

TEST_CASE("Plane::distanceTo")
{
	// (2, 3, 6, 7) is a Pythagorean quadruple.
	const seir::Plane plane{ { 1, 1.5f, 3 }, { 2, 3, 6 } };
	CHECK(plane.distanceTo({ 0, 0, 0 }) == -7);
	CHECK(plane.distanceTo({ 2, 3, 6 }) == 0);
	CHECK(plane.distanceTo({ 4, 6, 12 }) == 7);
}
