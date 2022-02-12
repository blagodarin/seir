// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

namespace seir
{
	//
	class Vec2
	{
	public:
		float x, y;

		Vec2() noexcept = default;
		constexpr Vec2(float vx, float vy) noexcept
			: x{ vx }, y{ vy } {}

		// clang-format off
		constexpr auto& operator+=(const Vec2& v) noexcept { x += v.x; y += v.y; return *this; }
		constexpr auto& operator+=(float s) noexcept { x += s; y += s; return *this; }
		constexpr auto& operator-=(const Vec2& v) noexcept { x -= v.x; y -= v.y; return *this; }
		constexpr auto& operator-=(float s) noexcept { x -= s; y -= s; return *this; }
		constexpr auto& operator*=(const Vec2& v) noexcept { x *= v.x; y *= v.y; return *this; }
		constexpr auto& operator*=(float s) noexcept { x *= s; y *= s; return *this; }
		constexpr auto& operator/=(const Vec2& v) noexcept { x /= v.x; y /= v.y; return *this; }
		constexpr auto& operator/=(float s) noexcept { x /= s; y /= s; return *this; }
		// clang-format on
	};

	//
	class Vec3
	{
	public:
		float x, y, z;

		Vec3() noexcept = default;
		constexpr Vec3(float vx, float vy, float vz) noexcept
			: x{ vx }, y{ vy }, z{ vz } {}

		// clang-format off
		constexpr auto& operator+=(const Vec3& v) noexcept { x += v.x; y += v.y; z += v.z; return *this; }
		constexpr auto& operator+=(float s) noexcept { x += s; y += s; z += s; return *this; }
		constexpr auto& operator-=(const Vec3& v) noexcept { x -= v.x; y -= v.y; z -= v.z; return *this; }
		constexpr auto& operator-=(float s) noexcept { x -= s; y -= s; z -= s; return *this; }
		constexpr auto& operator*=(const Vec3& v) noexcept { x *= v.x; y *= v.y; z *= v.z; return *this; }
		constexpr auto& operator*=(float s) noexcept { x *= s; y *= s; z *= s; return *this; }
		constexpr auto& operator/=(const Vec3& v) noexcept { x /= v.x; y /= v.y; z /= v.z; return *this; }
		constexpr auto& operator/=(float s) noexcept { x /= s; y /= s; z /= s; return *this; }
		// clang-format on
	};

	//
	class Vec4
	{
	public:
		float x, y, z, w;

		Vec4() noexcept = default;
		constexpr Vec4(float vx, float vy, float vz, float vw) noexcept
			: x{ vx }, y{ vy }, z{ vz }, w{ vw } {}

		// clang-format off
		constexpr auto& operator+=(const Vec4& v) noexcept { x += v.x; y += v.y; z += v.z; w += v.w; return *this; }
		constexpr auto& operator+=(float s) noexcept { x += s; y += s; z += s; w += s; return *this; }
		constexpr auto& operator-=(const Vec4& v) noexcept { x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }
		constexpr auto& operator-=(float s) noexcept { x -= s; y -= s; z -= s; w -= s; return *this; }
		constexpr auto& operator*=(const Vec4& v) noexcept { x *= v.x; y *= v.y; z *= v.z; w *= v.w; return *this; }
		constexpr auto& operator*=(float s) noexcept { x *= s; y *= s; z *= s; w *= s; return *this; }
		constexpr auto& operator/=(const Vec4& v) noexcept { x /= v.x; y /= v.y; z /= v.z; w /= v.w; return *this; }
		constexpr auto& operator/=(float s) noexcept { x /= s; y /= s; z /= s; w /= s; return *this; }
		// clang-format on
	};

	[[nodiscard]] constexpr bool operator==(const Vec2& a, const Vec2& b) noexcept { return a.x == b.x && a.y == b.y; }
	[[nodiscard]] constexpr bool operator==(const Vec3& a, const Vec3& b) noexcept { return a.x == b.x && a.y == b.y && a.z == b.z; }
	[[nodiscard]] constexpr bool operator==(const Vec4& a, const Vec4& b) noexcept { return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w; }

	[[nodiscard]] constexpr Vec2 operator+(const Vec2& a, const Vec2& b) noexcept { return { a.x + b.x, a.y + b.y }; }
	[[nodiscard]] constexpr Vec3 operator+(const Vec3& a, const Vec3& b) noexcept { return { a.x + b.x, a.y + b.y, a.z + b.z }; }
	[[nodiscard]] constexpr Vec4 operator+(const Vec4& a, const Vec4& b) noexcept { return { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w }; }

	[[nodiscard]] constexpr Vec2 operator+(const Vec2& a, float b) noexcept { return { a.x + b, a.y + b }; }
	[[nodiscard]] constexpr Vec3 operator+(const Vec3& a, float b) noexcept { return { a.x + b, a.y + b, a.z + b }; }
	[[nodiscard]] constexpr Vec4 operator+(const Vec4& a, float b) noexcept { return { a.x + b, a.y + b, a.z + b, a.w + b }; }

	[[nodiscard]] constexpr Vec2 operator+(float a, const Vec2& b) noexcept { return b + a; }
	[[nodiscard]] constexpr Vec3 operator+(float a, const Vec3& b) noexcept { return b + a; }
	[[nodiscard]] constexpr Vec4 operator+(float a, const Vec4& b) noexcept { return b + a; }

	[[nodiscard]] constexpr Vec2 operator-(const Vec2& v) noexcept { return { -v.x, -v.y }; }
	[[nodiscard]] constexpr Vec3 operator-(const Vec3& v) noexcept { return { -v.x, -v.y, -v.z }; }
	[[nodiscard]] constexpr Vec4 operator-(const Vec4& v) noexcept { return { -v.x, -v.y, -v.z, -v.w }; }

	[[nodiscard]] constexpr Vec2 operator-(const Vec2& a, const Vec2& b) noexcept { return { a.x - b.x, a.y - b.y }; }
	[[nodiscard]] constexpr Vec3 operator-(const Vec3& a, const Vec3& b) noexcept { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
	[[nodiscard]] constexpr Vec4 operator-(const Vec4& a, const Vec4& b) noexcept { return { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w }; }

	[[nodiscard]] constexpr Vec2 operator-(const Vec2& a, float b) noexcept { return { a.x - b, a.y - b }; }
	[[nodiscard]] constexpr Vec3 operator-(const Vec3& a, float b) noexcept { return { a.x - b, a.y - b, a.z - b }; }
	[[nodiscard]] constexpr Vec4 operator-(const Vec4& a, float b) noexcept { return { a.x - b, a.y - b, a.z - b, a.w - b }; }

	[[nodiscard]] constexpr Vec2 operator-(float a, const Vec2& b) noexcept { return { a - b.x, a - b.y }; }
	[[nodiscard]] constexpr Vec3 operator-(float a, const Vec3& b) noexcept { return { a - b.x, a - b.y, a - b.z }; }
	[[nodiscard]] constexpr Vec4 operator-(float a, const Vec4& b) noexcept { return { a - b.x, a - b.y, a - b.z, a - b.w }; }

	[[nodiscard]] constexpr Vec2 operator*(const Vec2& a, const Vec2& b) noexcept { return { a.x * b.x, a.y * b.y }; }
	[[nodiscard]] constexpr Vec3 operator*(const Vec3& a, const Vec3& b) noexcept { return { a.x * b.x, a.y * b.y, a.z * b.z }; }
	[[nodiscard]] constexpr Vec4 operator*(const Vec4& a, const Vec4& b) noexcept { return { a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w }; }

	[[nodiscard]] constexpr Vec2 operator*(const Vec2& a, float b) noexcept { return { a.x * b, a.y * b }; }
	[[nodiscard]] constexpr Vec3 operator*(const Vec3& a, float b) noexcept { return { a.x * b, a.y * b, a.z * b }; }
	[[nodiscard]] constexpr Vec4 operator*(const Vec4& a, float b) noexcept { return { a.x * b, a.y * b, a.z * b, a.w * b }; }

	[[nodiscard]] constexpr Vec2 operator*(float a, const Vec2& b) noexcept { return b * a; }
	[[nodiscard]] constexpr Vec3 operator*(float a, const Vec3& b) noexcept { return b * a; }
	[[nodiscard]] constexpr Vec4 operator*(float a, const Vec4& b) noexcept { return b * a; }

	[[nodiscard]] constexpr Vec2 operator/(const Vec2& a, const Vec2& b) noexcept { return { a.x / b.x, a.y / b.y }; }
	[[nodiscard]] constexpr Vec3 operator/(const Vec3& a, const Vec3& b) noexcept { return { a.x / b.x, a.y / b.y, a.z / b.z }; }
	[[nodiscard]] constexpr Vec4 operator/(const Vec4& a, const Vec4& b) noexcept { return { a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w }; }

	[[nodiscard]] constexpr Vec2 operator/(const Vec2& a, float b) noexcept { return { a.x / b, a.y / b }; }
	[[nodiscard]] constexpr Vec3 operator/(const Vec3& a, float b) noexcept { return { a.x / b, a.y / b, a.z / b }; }
	[[nodiscard]] constexpr Vec4 operator/(const Vec4& a, float b) noexcept { return { a.x / b, a.y / b, a.z / b, a.w / b }; }

	[[nodiscard]] constexpr Vec2 operator/(float a, const Vec2& b) noexcept { return { a / b.x, a / b.y }; }
	[[nodiscard]] constexpr Vec3 operator/(float a, const Vec3& b) noexcept { return { a / b.x, a / b.y, a / b.z }; }
	[[nodiscard]] constexpr Vec4 operator/(float a, const Vec4& b) noexcept { return { a / b.x, a / b.y, a / b.z, a / b.w }; }

	[[nodiscard]] constexpr auto dotProduct(const Vec3& a, const Vec3& b) noexcept { return a.x * b.x + a.y * b.y + a.z * b.z; }
}
