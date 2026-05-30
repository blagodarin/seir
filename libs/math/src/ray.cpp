// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_math/ray.hpp>

#include <seir_math/plane.hpp>

#include <cmath>

namespace seir
{
	std::optional<seir::Vec3> Ray3D::intersection(const Plane& plane) const noexcept
	{
		const auto s = dotProduct(_direction, plane.normal());
		if (std::abs(s) < 1e-6f)
			return {};
		const auto u = plane.distanceTo(_origin) / -s;
		if (u < 0)
			return {};
		return _origin + u * _direction;
	}
}
