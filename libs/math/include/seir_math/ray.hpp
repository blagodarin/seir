// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_math/vec.hpp>

#include <optional>

namespace seir
{
	class Plane;

	class Ray3D
	{
	public:
		[[nodiscard]] static constexpr Ray3D fromPoints(const Vec3& origin, const Vec3& target) noexcept;

		constexpr Ray3D() noexcept = default;

		[[nodiscard]] constexpr const Vec3& direction() const noexcept { return _direction; }
		[[nodiscard]] std::optional<Vec3> intersection(const Plane&) const noexcept;
		[[nodiscard]] constexpr const Vec3& origin() const noexcept { return _origin; }

	private:
		Vec3 _origin;
		Vec3 _direction;
		constexpr Ray3D(const Vec3& origin, const Vec3& direction) noexcept
			: _origin{ origin }, _direction{ direction } {}
	};
}

constexpr seir::Ray3D seir::Ray3D::fromPoints(const Vec3& origin, const Vec3& target) noexcept
{
	return { origin, target - origin };
}
