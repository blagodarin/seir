// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_math/mat.hpp>

#include <seir_math/euler.hpp>

#include <doctest/doctest.h>

namespace seir
{
	[[nodiscard]] constexpr bool operator==(const Mat4& a, const Mat4& b) noexcept { return a.x == b.x && a.y == b.y && a.z == b.z && a.t == b.t; }
}

using seir::Mat4;

namespace
{
	void checkEqual(const Mat4& actual, const Mat4& expected)
	{
		using doctest::Approx;
		CHECK(actual.x.x == Approx{ expected.x.x });
		CHECK(actual.x.y == Approx{ expected.x.y });
		CHECK(actual.x.z == Approx{ expected.x.z });
		CHECK(actual.x.w == Approx{ expected.x.w });
		CHECK(actual.y.x == Approx{ expected.y.x });
		CHECK(actual.y.y == Approx{ expected.y.y });
		CHECK(actual.y.z == Approx{ expected.y.z });
		CHECK(actual.y.w == Approx{ expected.y.w });
		CHECK(actual.z.x == Approx{ expected.z.x });
		CHECK(actual.z.y == Approx{ expected.z.y });
		CHECK(actual.z.z == Approx{ expected.z.z });
		CHECK(actual.z.w == Approx{ expected.z.w });
		CHECK(actual.t.x == Approx{ expected.t.x });
		CHECK(actual.t.y == Approx{ expected.t.y });
		CHECK(actual.t.z == Approx{ expected.t.z });
		CHECK(actual.t.w == Approx{ expected.t.w });
	}
}

TEST_CASE("Mat4(float...)")
{
	constexpr Mat4 m{
		11, 12, 13, 14,
		21, 22, 23, 24,
		31, 32, 33, 34,
		41, 42, 43, 44
	};
	CHECK(m.x == seir::Vec4(11, 21, 31, 41));
	CHECK(m.y == seir::Vec4(12, 22, 32, 42));
	CHECK(m.z == seir::Vec4(13, 23, 33, 43));
	CHECK(m.t == seir::Vec4(14, 24, 34, 44));
}

TEST_CASE("Mat4(Euler)")
{
	::checkEqual(Mat4{ seir::Euler{ 30, 45, 60 } },
		Mat4::rotation(30, { 0, 0, -1 }) * Mat4::rotation(45, { 1, 0, 0 }) * Mat4::rotation(60, { 0, 1, 0 }));
}

TEST_CASE("Mat4::camera")
{
	::checkEqual(Mat4::camera({ 1, 2, 3 }, { 30, 45, 60 }),
		Mat4::rotation(60, { 0, -1, 0 }) * Mat4::rotation(45, { -1, 0, 0 }) * Mat4::rotation(30, { 0, 0, 1 }) * Mat4::translation({ -1, -2, -3 }));
}

TEST_CASE("Mat4::projection2D")
{
	using seir::Vec3;
	constexpr auto m = Mat4::projection2D(640, 480, .75);
	CHECK(m * Vec3(0, 0, 0) == Vec3(-1, -1, 1));
	CHECK(m * Vec3(0, 0, .75) == Vec3(-1, -1, 0));
	CHECK(m * Vec3(640, 0, 0) == Vec3(1, -1, 1));
	CHECK(m * Vec3(640, 0, .75) == Vec3(1, -1, 0));
	CHECK(m * Vec3(0, 480, 0) == Vec3(-1, 1, 1));
	CHECK(m * Vec3(0, 480, .75) == Vec3(-1, 1, 0));
	CHECK(m * Vec3(640, 480, 0) == Vec3(1, 1, 1));
	CHECK(m * Vec3(640, 480, .75) == Vec3(1, 1, 0));
}

TEST_CASE("Mat4::projection3D")
{
	using seir::Vec3;
	const auto m = Mat4::projection3D(1, 90, 1);
	SUBCASE("center")
	{
		CHECK(m * Vec3(0, 1, 0) == Vec3(0, 0, 1));
		CHECK(m * Vec3(0, 2, 0) == Vec3(0, 0, .5));
		CHECK(m * Vec3(0, 4, 0) == Vec3(0, 0, .25));
	}
	SUBCASE("top left")
	{
		CHECK(m * Vec3(-1, 1, 1) == Vec3(-1, -1, 1));
		CHECK(m * Vec3(-2, 2, 2) == Vec3(-1, -1, .5));
		CHECK(m * Vec3(-4, 4, 4) == Vec3(-1, -1, .25));
	}
	SUBCASE("top right")
	{
		CHECK(m * Vec3(1, 1, 1) == Vec3(1, -1, 1));
		CHECK(m * Vec3(2, 2, 2) == Vec3(1, -1, .5));
		CHECK(m * Vec3(4, 4, 4) == Vec3(1, -1, .25));
	}
	SUBCASE("bottom left")
	{
		CHECK(m * Vec3(-1, 1, -1) == Vec3(-1, 1, 1));
		CHECK(m * Vec3(-2, 2, -2) == Vec3(-1, 1, .5));
		CHECK(m * Vec3(-4, 4, -4) == Vec3(-1, 1, .25));
	}
	SUBCASE("bottom right")
	{
		CHECK(m * Vec3(1, 1, -1) == Vec3(1, 1, 1));
		CHECK(m * Vec3(2, 2, -2) == Vec3(1, 1, .5));
		CHECK(m * Vec3(4, 4, -4) == Vec3(1, 1, .25));
	}
}

TEST_CASE("det")
{
	// Determinant "can be viewed as the scaling factor of the transformation described by the matrix" (c) Wikipedia.
	CHECK(det(Mat4::identity()) == 1.f);
	CHECK(det(Mat4::scaling(2)) == 2.f * 2.f * 2.f);
	CHECK(det(Mat4::translation({ 2, 3, 5 })) == 1.f);
	CHECK(det(Mat4::rotation(37, { 2, 3, 5 })) == doctest::Approx{ 1.f });
	CHECK(det(Mat4::scaling(4) * Mat4::rotation(37, { 2, 3, 5 }) * Mat4::scaling(2) * Mat4::translation({ 2, 3, 5 })) == doctest::Approx{ 4.f * 4.f * 4.f * 2.f * 2.f * 2.f });
}

TEST_CASE("inverse")
{
	CHECK(inverse(Mat4::identity()) == Mat4::identity());
	CHECK(inverse(Mat4::translation({ 2, 3, 5 })) == Mat4::translation({ -2, -3, -5 }));
	CHECK(inverse(Mat4::scaling(4)) == Mat4::scaling(0.25));
	::checkEqual(inverse(Mat4::rotation(37, { 2, 3, 5 })),
		Mat4::rotation(-37, { 2, 3, 5 }));
	::checkEqual(inverse(Mat4::scaling(4) * Mat4::rotation(37, { 2, 3, 5 }) * Mat4::scaling(2) * Mat4::translation({ 2, 3, 5 })),
		Mat4::translation({ -2, -3, -5 }) * Mat4::scaling(0.5) * Mat4::rotation(-37, { 2, 3, 5 }) * Mat4::scaling(0.25));
}
