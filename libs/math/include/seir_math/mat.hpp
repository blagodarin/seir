// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_math/vec.hpp>

namespace seir
{
	class Euler;

	//
	class Mat4
	{
	public:
		Vec4 x, y, z, t;

		Mat4() noexcept = default;
		constexpr Mat4(float xx, float yx, float zx, float tx, float xy, float yy, float zy, float ty, float xz, float yz, float zz, float tz, float xw, float yw, float zw, float tw) noexcept
			: x{ xx, xy, xz, xw }, y{ yx, yy, yz, yw }, z{ zx, zy, zz, zw }, t{ tx, ty, tz, tw } {}
		explicit Mat4(const Euler&) noexcept;

		[[nodiscard]] static inline Mat4 camera(const Vec3& position, const Euler& orientation) noexcept;
		[[nodiscard]] static constexpr Mat4 identity() noexcept;
		[[nodiscard]] static Mat4 perspective(float width, float height, float verticalFov, float nearPlane, float farPlane) noexcept;
		[[nodiscard]] static constexpr Mat4 projection2D(float width, float height, float nearPlane = -1.f, float farPlane = 1.f) noexcept;
		[[nodiscard]] static Mat4 rotation(float degrees, const Vec3& axis) noexcept;
		[[nodiscard]] static constexpr Mat4 scaling(float) noexcept;
		[[nodiscard]] static constexpr Mat4 translation(const Vec3&) noexcept;
	};

	[[nodiscard]] constexpr Mat4 operator*(const Mat4&, const Mat4&) noexcept;
	[[nodiscard]] constexpr Vec4 operator*(const Mat4&, const Vec4&) noexcept;
	[[nodiscard]] constexpr Vec3 operator*(const Mat4&, const Vec3&) noexcept;

	[[nodiscard]] constexpr float det(const Mat4&) noexcept;
	[[nodiscard]] constexpr Mat4 inverse(const Mat4&) noexcept;
}

seir::Mat4 seir::Mat4::camera(const Vec3& position, const Euler& orientation) noexcept
{
	const Mat4 r{ orientation };
	return {
		r.x.x, r.x.y, r.x.z, -dotProduct(position, { r.x.x, r.x.y, r.x.z }),
		r.y.x, r.y.y, r.y.z, -dotProduct(position, { r.y.x, r.y.y, r.y.z }),
		r.z.x, r.z.y, r.z.z, -dotProduct(position, { r.z.x, r.z.y, r.z.z }),
		0, 0, 0, 1
	};
}

constexpr seir::Mat4 seir::Mat4::identity() noexcept
{
	return {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
}

constexpr seir::Mat4 seir::Mat4::projection2D(float width, float height, float nearPlane, float farPlane) noexcept
{
	const auto xx = 2 / width;
	const auto yy = -2 / height;
	const auto zz = -2 / (farPlane - nearPlane);
	const auto tx = -1.f;
	const auto ty = 1.f;
	const auto tz = (farPlane + nearPlane) / (farPlane - nearPlane);
	return {
		xx, 0, 0, tx,
		0, yy, 0, ty,
		0, 0, zz, tz,
		0, 0, 0, 1
	};
}

constexpr seir::Mat4 seir::Mat4::scaling(float s) noexcept
{
	return {
		s, 0, 0, 0,
		0, s, 0, 0,
		0, 0, s, 0,
		0, 0, 0, 1
	};
}

constexpr seir::Mat4 seir::Mat4::translation(const Vec3& v) noexcept
{
	return {
		1, 0, 0, v.x,
		0, 1, 0, v.y,
		0, 0, 1, v.z,
		0, 0, 0, 1
	};
}

constexpr seir::Mat4 seir::operator*(const Mat4& a, const Mat4& b) noexcept
{
	return {
		a.x.x * b.x.x + a.y.x * b.x.y + a.z.x * b.x.z + a.t.x * b.x.w,
		a.x.x * b.y.x + a.y.x * b.y.y + a.z.x * b.y.z + a.t.x * b.y.w,
		a.x.x * b.z.x + a.y.x * b.z.y + a.z.x * b.z.z + a.t.x * b.z.w,
		a.x.x * b.t.x + a.y.x * b.t.y + a.z.x * b.t.z + a.t.x * b.t.w,

		a.x.y * b.x.x + a.y.y * b.x.y + a.z.y * b.x.z + a.t.y * b.x.w,
		a.x.y * b.y.x + a.y.y * b.y.y + a.z.y * b.y.z + a.t.y * b.y.w,
		a.x.y * b.z.x + a.y.y * b.z.y + a.z.y * b.z.z + a.t.y * b.z.w,
		a.x.y * b.t.x + a.y.y * b.t.y + a.z.y * b.t.z + a.t.y * b.t.w,

		a.x.z * b.x.x + a.y.z * b.x.y + a.z.z * b.x.z + a.t.z * b.x.w,
		a.x.z * b.y.x + a.y.z * b.y.y + a.z.z * b.y.z + a.t.z * b.y.w,
		a.x.z * b.z.x + a.y.z * b.z.y + a.z.z * b.z.z + a.t.z * b.z.w,
		a.x.z * b.t.x + a.y.z * b.t.y + a.z.z * b.t.z + a.t.z * b.t.w,

		a.x.w * b.x.x + a.y.w * b.x.y + a.z.w * b.x.z + a.t.w * b.x.w,
		a.x.w * b.y.x + a.y.w * b.y.y + a.z.w * b.y.z + a.t.w * b.y.w,
		a.x.w * b.z.x + a.y.w * b.z.y + a.z.w * b.z.z + a.t.w * b.z.w,
		a.x.w * b.t.x + a.y.w * b.t.y + a.z.w * b.t.z + a.t.w * b.t.w,
	};
}

constexpr seir::Vec4 seir::operator*(const Mat4& m, const Vec4& v) noexcept
{
	return {
		m.x.x * v.x + m.y.x * v.y + m.z.x * v.z + m.t.x * v.w,
		m.x.y * v.x + m.y.y * v.y + m.z.y * v.z + m.t.y * v.w,
		m.x.z * v.x + m.y.z * v.y + m.z.z * v.z + m.t.z * v.w,
		m.x.w * v.x + m.y.w * v.y + m.z.w * v.z + m.t.w * v.w,
	};
}

constexpr seir::Vec3 seir::operator*(const Mat4& m, const Vec3& v) noexcept
{
	const auto w = m.x.w * v.x + m.y.w * v.y + m.z.w * v.z + m.t.w;
	return {
		(m.x.x * v.x + m.y.x * v.y + m.z.x * v.z + m.t.x) / w,
		(m.x.y * v.x + m.y.y * v.y + m.z.y * v.z + m.t.y) / w,
		(m.x.z * v.x + m.y.z * v.y + m.z.z * v.z + m.t.z) / w,
	};
}

constexpr float seir::det(const Mat4& m) noexcept
{
	const auto xy = m.x.z * m.y.w - m.x.w * m.y.z;
	const auto xz = m.x.z * m.z.w - m.x.w * m.z.z;
	const auto xt = m.x.z * m.t.w - m.x.w * m.t.z;
	const auto yz = m.y.z * m.z.w - m.y.w * m.z.z;
	const auto yt = m.y.z * m.t.w - m.y.w * m.t.z;
	const auto zt = m.z.z * m.t.w - m.z.w * m.t.z;
	const auto yzt = m.y.y * zt - m.z.y * yt + m.t.y * yz;
	const auto xzt = m.x.y * zt - m.z.y * xt + m.t.y * xz;
	const auto xyt = m.x.y * yt - m.y.y * xt + m.t.y * xy;
	const auto xyz = m.x.y * yz - m.y.y * xz + m.z.y * xy;
	return m.x.x * yzt - m.y.x * xzt + m.z.x * xyt - m.t.x * xyz;
}

constexpr seir::Mat4 seir::inverse(const Mat4& m) noexcept
{
	// Z and W rows.
	auto det01 = m.x.z * m.y.w - m.x.w * m.y.z;
	auto det02 = m.x.z * m.z.w - m.x.w * m.z.z;
	auto det03 = m.x.z * m.t.w - m.x.w * m.t.z;
	auto det12 = m.y.z * m.z.w - m.y.w * m.z.z;
	auto det13 = m.y.z * m.t.w - m.y.w * m.t.z;
	auto det23 = m.z.z * m.t.w - m.z.w * m.t.z;

	// Y, Z and W rows.
	const auto det123 = m.y.y * det23 - m.z.y * det13 + m.t.y * det12;
	const auto det023 = m.x.y * det23 - m.z.y * det03 + m.t.y * det02;
	const auto det013 = m.x.y * det13 - m.y.y * det03 + m.t.y * det01;
	const auto det012 = m.x.y * det12 - m.y.y * det02 + m.z.y * det01;

	const auto d = 1 / (m.x.x * det123 - m.y.x * det023 + m.z.x * det013 - m.t.x * det012);

	const auto xx = d * det123;
	const auto xy = d * -det023;
	const auto xz = d * det013;
	const auto xw = d * -det012;

	const auto yx = d * -(m.y.x * det23 - m.z.x * det13 + m.t.x * det12);
	const auto yy = d * (m.x.x * det23 - m.z.x * det03 + m.t.x * det02);
	const auto yz = d * -(m.x.x * det13 - m.y.x * det03 + m.t.x * det01);
	const auto yw = d * (m.x.x * det12 - m.y.x * det02 + m.z.x * det01);

	// Y and W rows.
	det01 = m.x.y * m.y.w - m.y.y * m.x.w;
	det02 = m.x.y * m.z.w - m.z.y * m.x.w;
	det03 = m.x.y * m.t.w - m.t.y * m.x.w;
	det12 = m.y.y * m.z.w - m.z.y * m.y.w;
	det13 = m.y.y * m.t.w - m.t.y * m.y.w;
	det23 = m.z.y * m.t.w - m.t.y * m.z.w;

	const auto zx = d * (m.y.x * det23 - m.z.x * det13 + m.t.x * det12);
	const auto zy = d * -(m.x.x * det23 - m.z.x * det03 + m.t.x * det02);
	const auto zz = d * (m.x.x * det13 - m.y.x * det03 + m.t.x * det01);
	const auto zw = d * -(m.x.x * det12 - m.y.x * det02 + m.z.x * det01);

	// Y and Z rows.
	det01 = m.y.z * m.x.y - m.x.z * m.y.y;
	det02 = m.z.z * m.x.y - m.x.z * m.z.y;
	det03 = m.t.z * m.x.y - m.x.z * m.t.y;
	det12 = m.z.z * m.y.y - m.y.z * m.z.y;
	det13 = m.t.z * m.y.y - m.y.z * m.t.y;
	det23 = m.t.z * m.z.y - m.z.z * m.t.y;

	const auto tx = d * -(m.y.x * det23 - m.z.x * det13 + m.t.x * det12);
	const auto ty = d * (m.x.x * det23 - m.z.x * det03 + m.t.x * det02);
	const auto tz = d * -(m.x.x * det13 - m.y.x * det03 + m.t.x * det01);
	const auto tw = d * (m.x.x * det12 - m.y.x * det02 + m.z.x * det01);

	return {
		xx, yx, zx, tx,
		xy, yy, zy, ty,
		xz, yz, zz, tz,
		xw, yw, zw, tw
	};
}
