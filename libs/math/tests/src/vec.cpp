// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_math/vec.hpp>

#include <doctest/doctest.h>

TEST_CASE("Vec2")
{
	using seir::Vec2;

	CHECK(Vec2(1, 2) + Vec2(3, 4) == Vec2(4, 6));
	CHECK(Vec2(1, 2) + 3 == Vec2(4, 5));
	CHECK(1 + Vec2(2, 3) == Vec2(3, 4));

	CHECK(-Vec2(1, 2) == Vec2(-1, -2));

	CHECK(Vec2(4, 3) - Vec2(2, 1) == Vec2(2, 2));
	CHECK(Vec2(5, 4) - 3 == Vec2(2, 1));
	CHECK(5 - Vec2(4, 3) == Vec2(1, 2));

	CHECK(Vec2(1, 2) * Vec2(3, 4) == Vec2(3, 8));
	CHECK(Vec2(1, 2) * 3 == Vec2(3, 6));
	CHECK(2 * Vec2(3, 4) == Vec2(6, 8));

	CHECK(Vec2(6, 8) / Vec2(3, 2) == Vec2(2, 4));
	CHECK(Vec2(4, 6) / 2 == Vec2(2, 3));
	CHECK(6 / Vec2(2, 3) == Vec2(3, 2));
}

TEST_CASE("Vec3")
{
	using seir::Vec3;

	CHECK(Vec3(1, 2, 3) + Vec3(4, 5, 6) == Vec3(5, 7, 9));
	CHECK(Vec3(1, 2, 3) + 4 == Vec3(5, 6, 7));
	CHECK(1 + Vec3(2, 3, 4) == Vec3(3, 4, 5));

	CHECK(-Vec3(1, 2, 3) == Vec3(-1, -2, -3));

	CHECK(Vec3(6, 5, 4) - Vec3(3, 2, 1) == Vec3(3, 3, 3));
	CHECK(Vec3(6, 5, 4) - 3 == Vec3(3, 2, 1));
	CHECK(7 - Vec3(6, 5, 4) == Vec3(1, 2, 3));

	CHECK(Vec3(1, 2, 3) * Vec3(4, 5, 6) == Vec3(4, 10, 18));
	CHECK(Vec3(1, 2, 3) * 4 == Vec3(4, 8, 12));
	CHECK(4 * Vec3(3, 2, 1) == Vec3(12, 8, 4));

	CHECK(Vec3(1, 4, 9) / Vec3(1, 2, 3) == Vec3(1, 2, 3));
	CHECK(Vec3(2, 4, 6) / 2 == Vec3(1, 2, 3));
	CHECK(4 / Vec3(1, 2, 4) == Vec3(4, 2, 1));
}

TEST_CASE("Vec4")
{
	using seir::Vec4;

	CHECK(Vec4(1, 2, 3, 4) + Vec4(5, 6, 7, 8) == Vec4(6, 8, 10, 12));
	CHECK(Vec4(1, 2, 3, 4) + 5 == Vec4(6, 7, 8, 9));
	CHECK(1 + Vec4(2, 3, 4, 5) == Vec4(3, 4, 5, 6));

	CHECK(-Vec4(1, 2, 3, 4) == Vec4(-1, -2, -3, -4));

	CHECK(Vec4(8, 7, 6, 5) - Vec4(4, 3, 2, 1) == Vec4(4, 4, 4, 4));
	CHECK(Vec4(9, 8, 7, 6) - 5 == Vec4(4, 3, 2, 1));
	CHECK(9 - Vec4(8, 7, 6, 5) == Vec4(1, 2, 3, 4));

	CHECK(Vec4(1, 2, 3, 4) * Vec4(5, 6, 7, 8) == Vec4(5, 12, 21, 32));
	CHECK(Vec4(1, 2, 3, 4) * 5 == Vec4(5, 10, 15, 20));
	CHECK(5 * Vec4(4, 3, 2, 1) == Vec4(20, 15, 10, 5));

	CHECK(Vec4(1, 4, 9, 16) / Vec4(1, 2, 3, 4) == Vec4(1, 2, 3, 4));
	CHECK(Vec4(2, 4, 6, 8) / 2 == Vec4(1, 2, 3, 4));
	CHECK(8 / Vec4(1, 2, 4, 8) == Vec4(8, 4, 2, 1));
}

TEST_CASE("dotProduct")
{
	CHECK(seir::dotProduct({ 1.f, 10.f, 100.f }, { 2.f, 3.f, 4.f }) == 432.f);
}

TEST_CASE("normalize")
{
	const seir::Vec3 v{ 1, 4, 8 };
	const auto n = seir::normalize(v);
	CHECK(n == v / 9);
}
