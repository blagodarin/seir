// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_math/vec.hpp>

namespace seir
{
	class Plane
	{
	public:
		constexpr Plane() noexcept = default;
		Plane(const Vec3& normal, const Vec3& origin) noexcept
			: _normal{ normalize(normal) }, _offset{ dotProduct(_normal, origin) } {}

		[[nodiscard]] constexpr float distanceTo(const Vec3& point) const noexcept { return dotProduct(_normal, point) - _offset; }
		[[nodiscard]] constexpr const Vec3& normal() const noexcept { return _normal; }

	private:
		Vec3 _normal;
		float _offset = 0; // Distance from the origin to the plane (along the normal).
	};
}
