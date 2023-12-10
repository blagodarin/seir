// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_math/vec.hpp>

#include <optional>

namespace seir
{
	class Plane;

	class Line3
	{
	public:
		Vec3 _origin;
		Vec3 _vector;

		Line3() noexcept = default;
		constexpr Line3(const Vec3& from, const Vec3& to) noexcept
			: _origin{ from }, _vector{ to - from } {}

		[[nodiscard]] std::optional<Vec3> intersection(const Plane&) const noexcept;
	};
}
