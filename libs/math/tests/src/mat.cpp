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
	const Mat4 actual{ seir::Euler{ 30, 45, 60 } };
	const auto expected = Mat4::rotation(30, { 0, 0, -1 }) * Mat4::rotation(45, { 1, 0, 0 }) * Mat4::rotation(60, { 0, 1, 0 });
	CHECK(actual.x.x == doctest::Approx{ expected.x.x }.epsilon(3e-5));
	CHECK(actual.x.y == doctest::Approx{ expected.x.y }.epsilon(3e-5));
	CHECK(actual.x.z == doctest::Approx{ expected.x.z }.epsilon(3e-5));
	CHECK(actual.x.w == doctest::Approx{ expected.x.w }.epsilon(3e-5));
	CHECK(actual.y.x == doctest::Approx{ expected.y.x }.epsilon(3e-5));
	CHECK(actual.y.y == doctest::Approx{ expected.y.y }.epsilon(3e-5));
	CHECK(actual.y.z == doctest::Approx{ expected.y.z }.epsilon(3e-5));
	CHECK(actual.y.w == doctest::Approx{ expected.y.w }.epsilon(3e-5));
	CHECK(actual.z.x == doctest::Approx{ expected.z.x }.epsilon(3e-5));
	CHECK(actual.z.y == doctest::Approx{ expected.z.y }.epsilon(3e-5));
	CHECK(actual.z.z == doctest::Approx{ expected.z.z }.epsilon(3e-5));
	CHECK(actual.z.w == doctest::Approx{ expected.z.w }.epsilon(3e-5));
	CHECK(actual.t.x == doctest::Approx{ expected.t.x }.epsilon(3e-5));
	CHECK(actual.t.y == doctest::Approx{ expected.t.y }.epsilon(3e-5));
	CHECK(actual.t.z == doctest::Approx{ expected.t.z }.epsilon(3e-5));
	CHECK(actual.t.w == doctest::Approx{ expected.t.w }.epsilon(3e-5));
}

TEST_CASE("Mat4::camera")
{
	const auto actual = Mat4::camera({ 1, 2, 3 }, { 30, 45, 60 });
	const auto expected = Mat4::rotation(60, { 0, -1, 0 }) * Mat4::rotation(45, { -1, 0, 0 }) * Mat4::rotation(30, { 0, 0, 1 }) * Mat4::translation({ -1, -2, -3 });
	CHECK(actual.x.x == doctest::Approx{ expected.x.x }.epsilon(3e-5));
	CHECK(actual.x.y == doctest::Approx{ expected.x.y }.epsilon(3e-5));
	CHECK(actual.x.z == doctest::Approx{ expected.x.z }.epsilon(3e-5));
	CHECK(actual.x.w == doctest::Approx{ expected.x.w }.epsilon(3e-5));
	CHECK(actual.y.x == doctest::Approx{ expected.y.x }.epsilon(3e-5));
	CHECK(actual.y.y == doctest::Approx{ expected.y.y }.epsilon(3e-5));
	CHECK(actual.y.z == doctest::Approx{ expected.y.z }.epsilon(3e-5));
	CHECK(actual.y.w == doctest::Approx{ expected.y.w }.epsilon(3e-5));
	CHECK(actual.z.x == doctest::Approx{ expected.z.x }.epsilon(3e-5));
	CHECK(actual.z.y == doctest::Approx{ expected.z.y }.epsilon(3e-5));
	CHECK(actual.z.z == doctest::Approx{ expected.z.z }.epsilon(3e-5));
	CHECK(actual.z.w == doctest::Approx{ expected.z.w }.epsilon(3e-5));
	CHECK(actual.t.x == doctest::Approx{ expected.t.x }.epsilon(3e-5));
	CHECK(actual.t.y == doctest::Approx{ expected.t.y }.epsilon(3e-5));
	CHECK(actual.t.z == doctest::Approx{ expected.t.z }.epsilon(3e-5));
	CHECK(actual.t.w == doctest::Approx{ expected.t.w }.epsilon(3e-5));
}

TEST_CASE("Mat4::projection2D")
{
	using doctest::Approx;
	using seir::Vec3;
	constexpr auto m = Mat4::projection2D(640, 480, .75);
	{
		constexpr auto v = m * Vec3{ 0, 0, 0 };
		CHECK(v.x == -1.f);
		CHECK(v.y == -1.f);
		CHECK(v.z == 1.f);
	}
	{
		constexpr auto v = m * Vec3{ 0, 0, .75 };
		CHECK(v.x == -1.f);
		CHECK(v.y == -1.f);
		CHECK(v.z == 0.f);
	}
	{
		constexpr auto v = m * Vec3{ 640, 0, 0 };
		CHECK(v.x == 1.f);
		CHECK(v.y == -1.f);
		CHECK(v.z == 1.f);
	}
	{
		constexpr auto v = m * Vec3{ 640, 0, .75 };
		CHECK(v.x == 1.f);
		CHECK(v.y == -1.f);
		CHECK(v.z == 0.f);
	}
	{
		constexpr auto v = m * Vec3{ 0, 480, 0 };
		CHECK(v.x == -1.f);
		CHECK(v.y == 1.f);
		CHECK(v.z == 1.f);
	}
	{
		constexpr auto v = m * Vec3{ 0, 480, .75 };
		CHECK(v.x == -1.f);
		CHECK(v.y == 1.f);
		CHECK(v.z == 0.f);
	}
	{
		constexpr auto v = m * Vec3{ 640, 480, 0 };
		CHECK(v.x == 1.f);
		CHECK(v.y == 1.f);
		CHECK(v.z == 1.f);
	}
	{
		constexpr auto v = m * Vec3{ 640, 480, .75 };
		CHECK(v.x == 1.f);
		CHECK(v.y == 1.f);
		CHECK(v.z == 0.f);
	}
}

TEST_CASE("Mat4::projection3D")
{
	using seir::Vec3;
	const auto m = Mat4::projection3D(1, 90, 1);
	{
		const auto v = m * Vec3{ 0, 1, 0 };
		CHECK(v.x == 0.f);
		CHECK(v.y == 0.f);
		CHECK(v.z == 1.f);
	}
	{
		const auto v = m * Vec3{ -1, 1, 1 };
		CHECK(v.x == -1.f);
		CHECK(v.y == -1.f);
		CHECK(v.z == 1.f);
	}
	{
		const auto v = m * Vec3{ 1, 1, 1 };
		CHECK(v.x == 1.f);
		CHECK(v.y == -1.f);
		CHECK(v.z == 1.f);
	}
	{
		const auto v = m * Vec3{ -1, 1, -1 };
		CHECK(v.x == -1.f);
		CHECK(v.y == 1.f);
		CHECK(v.z == 1.f);
	}
	{
		const auto v = m * Vec3{ 1, 1, -1 };
		CHECK(v.x == 1.f);
		CHECK(v.y == 1.f);
		CHECK(v.z == 1.f);
	}
	{
		const auto v = m * Vec3{ 0, 2, 0 };
		CHECK(v.x == 0.f);
		CHECK(v.y == 0.f);
		CHECK(v.z == .5f);
	}
	{
		const auto v = m * Vec3{ -2, 2, 2 };
		CHECK(v.x == -1.f);
		CHECK(v.y == -1.f);
		CHECK(v.z == .5f);
	}
	{
		const auto v = m * Vec3{ 2, 2, 2 };
		CHECK(v.x == 1.f);
		CHECK(v.y == -1.f);
		CHECK(v.z == .5f);
	}
	{
		const auto v = m * Vec3{ -2, 2, -2 };
		CHECK(v.x == -1.f);
		CHECK(v.y == 1.f);
		CHECK(v.z == .5f);
	}
	{
		const auto v = m * Vec3{ 2, 2, -2 };
		CHECK(v.x == 1.f);
		CHECK(v.y == 1.f);
		CHECK(v.z == .5f);
	}
}

TEST_CASE("det")
{
	// Determinant "can be viewed as the scaling factor of the transformation described by the matrix" (c) Wikipedia.
	CHECK(det(Mat4::identity()) == 1.f);
	CHECK(det(Mat4::scaling(2)) == 2.f * 2.f * 2.f);
	CHECK(det(Mat4::translation({ 2, 3, 5 })) == 1.f);
	CHECK(det(Mat4::rotation(37, { 2, 3, 5 })) == doctest::Approx{ 1.f }.epsilon(2e-5));
	CHECK(det(Mat4::scaling(4) * Mat4::rotation(37, { 2, 3, 5 }) * Mat4::scaling(2) * Mat4::translation({ 2, 3, 5 })) == doctest::Approx{ 4.f * 4.f * 4.f * 2.f * 2.f * 2.f }.epsilon(2e-5));
}

TEST_CASE("inverse")
{
	CHECK(inverse(Mat4::identity()) == Mat4::identity());
	CHECK(inverse(Mat4::translation({ 2, 3, 5 })) == Mat4::translation({ -2, -3, -5 }));
	CHECK(inverse(Mat4::scaling(4)) == Mat4::scaling(0.25));
	{
		const auto actual = inverse(Mat4::rotation(37, { 2, 3, 5 }));
		const auto expected = Mat4::rotation(-37, { 2, 3, 5 });
		CHECK(actual.x.x == doctest::Approx{ expected.x.x }.epsilon(2e-5));
		CHECK(actual.x.y == doctest::Approx{ expected.x.y }.epsilon(2e-5));
		CHECK(actual.x.z == doctest::Approx{ expected.x.z }.epsilon(2e-5));
		CHECK(actual.x.w == doctest::Approx{ expected.x.w }.epsilon(2e-5));
		CHECK(actual.y.x == doctest::Approx{ expected.y.x }.epsilon(2e-5));
		CHECK(actual.y.y == doctest::Approx{ expected.y.y }.epsilon(2e-5));
		CHECK(actual.y.z == doctest::Approx{ expected.y.z }.epsilon(2e-5));
		CHECK(actual.y.w == doctest::Approx{ expected.y.w }.epsilon(2e-5));
		CHECK(actual.z.x == doctest::Approx{ expected.z.x }.epsilon(2e-5));
		CHECK(actual.z.y == doctest::Approx{ expected.z.y }.epsilon(2e-5));
		CHECK(actual.z.z == doctest::Approx{ expected.z.z }.epsilon(2e-5));
		CHECK(actual.z.w == doctest::Approx{ expected.z.w }.epsilon(2e-5));
		CHECK(actual.t.x == doctest::Approx{ expected.t.x }.epsilon(2e-5));
		CHECK(actual.t.y == doctest::Approx{ expected.t.y }.epsilon(2e-5));
		CHECK(actual.t.z == doctest::Approx{ expected.t.z }.epsilon(2e-5));
		CHECK(actual.t.w == doctest::Approx{ expected.t.w }.epsilon(2e-5));
	}
	{
		const auto actual = inverse(Mat4::scaling(4) * Mat4::rotation(37, { 2, 3, 5 }) * Mat4::scaling(2) * Mat4::translation({ 2, 3, 5 }));
		const auto expected = Mat4::translation({ -2, -3, -5 }) * Mat4::scaling(0.5) * Mat4::rotation(-37, { 2, 3, 5 }) * Mat4::scaling(0.25);
		CHECK(actual.x.x == doctest::Approx{ expected.x.x }.epsilon(2e-5));
		CHECK(actual.x.y == doctest::Approx{ expected.x.y }.epsilon(2e-5));
		CHECK(actual.x.z == doctest::Approx{ expected.x.z }.epsilon(2e-5));
		CHECK(actual.x.w == doctest::Approx{ expected.x.w }.epsilon(2e-5));
		CHECK(actual.y.x == doctest::Approx{ expected.y.x }.epsilon(2e-5));
		CHECK(actual.y.y == doctest::Approx{ expected.y.y }.epsilon(2e-5));
		CHECK(actual.y.z == doctest::Approx{ expected.y.z }.epsilon(2e-5));
		CHECK(actual.y.w == doctest::Approx{ expected.y.w }.epsilon(2e-5));
		CHECK(actual.z.x == doctest::Approx{ expected.z.x }.epsilon(2e-5));
		CHECK(actual.z.y == doctest::Approx{ expected.z.y }.epsilon(2e-5));
		CHECK(actual.z.z == doctest::Approx{ expected.z.z }.epsilon(2e-5));
		CHECK(actual.z.w == doctest::Approx{ expected.z.w }.epsilon(2e-5));
		CHECK(actual.t.x == doctest::Approx{ expected.t.x }.epsilon(2e-5));
		CHECK(actual.t.y == doctest::Approx{ expected.t.y }.epsilon(2e-5));
		CHECK(actual.t.z == doctest::Approx{ expected.t.z }.epsilon(2e-5));
		CHECK(actual.t.w == doctest::Approx{ expected.t.w }.epsilon(2e-5));
	}
}
