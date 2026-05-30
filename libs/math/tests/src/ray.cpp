// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_math/ray.hpp>

#include <seir_math/plane.hpp>

#include <doctest/doctest.h>

TEST_CASE("Ray3D::intersection")
{
	// (2, 3, 6, 7) is a Pythagorean quadruple.
	const seir::Plane plane{ { 1, 1.5f, 3 }, { 2, 3, 6 } };
	SUBCASE("negative half-space, towards the plane")
	{
		const auto ray = seir::Ray3D::fromPoints({ 0, 0, 0 }, { 4, 6, 12 });
		const auto intersection = ray.intersection(plane);
		REQUIRE(intersection);
		CHECK(*intersection == seir::Vec3(2, 3, 6));
	}
	SUBCASE("negative half-space, away from the plane")
	{
		const auto ray = seir::Ray3D::fromPoints({ 0, 0, 0 }, { -1, -1, -1 });
		CHECK_FALSE(ray.intersection(plane));
	}
	SUBCASE("negative half-space, parallel to the plane")
	{
		const auto ray = seir::Ray3D::fromPoints({ 0, 0, 0 }, { 3, 4, -3 });
		REQUIRE(plane.distanceTo(ray.origin()) == -7);
		REQUIRE(plane.distanceTo(ray.origin() + ray.direction()) == -7);
		CHECK_FALSE(ray.intersection(plane));
	}
	SUBCASE("positive half-space, towards the plane")
	{
		const auto ray = seir::Ray3D::fromPoints({ 4, 6, 12 }, { 0, 0, 0 });
		const auto intersection = ray.intersection(plane);
		REQUIRE(intersection);
		CHECK(*intersection == seir::Vec3(2, 3, 6));
	}
	SUBCASE("positive half-space, away from the plane")
	{
		const auto ray = seir::Ray3D::fromPoints({ 4, 6, 12 }, { 5, 7, 13 });
		CHECK_FALSE(ray.intersection(plane));
	}
	SUBCASE("positive half-space, parallel to the plane")
	{
		const auto ray = seir::Ray3D::fromPoints({ 4, 6, 12 }, { 7, 10, 9 });
		REQUIRE(plane.distanceTo(ray.origin()) == 7);
		REQUIRE(plane.distanceTo(ray.origin() + ray.direction()) == 7);
		CHECK_FALSE(ray.intersection(plane));
	}
}
