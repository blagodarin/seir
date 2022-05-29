// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_math/mat.hpp>

#include <seir_math/euler.hpp>

#include <cmath>
#include <numbers>

namespace seir
{
	Mat4::Mat4(const Euler& e) noexcept // NOLINT(cppcoreguidelines-pro-type-member-init)
	{
		constexpr auto toRadians = std::numbers::pi_v<float> / 180;
		const auto yaw = e._yaw * toRadians;
		const auto pitch = e._pitch * toRadians;
		const auto roll = e._roll * toRadians;
		const auto cy = std::cos(yaw);
		const auto sy = std::sin(yaw);
		const auto cp = std::cos(pitch);
		const auto sp = std::sin(pitch);
		const auto cr = std::cos(roll);
		const auto sr = std::sin(roll);
		x = { sy * sp * sr + cy * cr, cy * sp * sr - sy * cr, -cp * sr, 0 };
		y = { sy * cp, cy * cp, sp, 0 };
		z = { cy * sr - sy * sp * cr, -cy * sp * cr - sy * sr, cp * cr, 0 };
		t = { 0, 0, 0, 1 };
	}

	Mat4 Mat4::projection3D(float aspectRatio, float verticalFov, float nearPlane) noexcept
	{
		const auto f = 1 / std::tan(verticalFov / 360 * std::numbers::pi_v<float>);
		const auto xx = f / aspectRatio;
		const auto yy = -f;
		const auto tz = nearPlane;
		return {
			xx, 0, 0, 0,
			0, 0, yy, 0,
			0, 0, 0, tz,
			0, 1, 0, 0
		};
	}

	Mat4 Mat4::rotation(float degrees, const Vec3& axis) noexcept
	{
		const auto v = normalize(axis);
		const auto radians = degrees / 180 * std::numbers::pi_v<float>;
		const auto c = std::cos(radians);
		const auto s = std::sin(radians);
		return {
			v.x * v.x * (1 - c) + c, v.y * v.x * (1 - c) - s * v.z, v.z * v.x * (1 - c) + s * v.y, 0,
			v.x * v.y * (1 - c) + s * v.z, v.y * v.y * (1 - c) + c, v.z * v.y * (1 - c) - s * v.x, 0,
			v.x * v.z * (1 - c) - s * v.y, v.y * v.z * (1 - c) + s * v.x, v.z * v.z * (1 - c) + c, 0,
			0, 0, 0, 1
		};
	}
}
