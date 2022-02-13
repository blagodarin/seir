// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_math/vec.hpp>

#include <cmath>

namespace seir
{
	Vec3 normalize(const Vec3& v) noexcept
	{
		return v / std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	}
}
