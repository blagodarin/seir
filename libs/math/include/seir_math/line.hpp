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
		constexpr Line3() noexcept = default;
		constexpr Line3(const Vec3& from, const Vec3& to) noexcept
			: _origin{ from }, _vector{ to - from } {}

		[[nodiscard]] constexpr const Vec3& first() const noexcept { return _origin; }
		[[nodiscard]] std::optional<Vec3> intersection(const Plane&) const noexcept;
		[[nodiscard]] constexpr Vec3 second() const noexcept { return _origin + _vector; }

	private:
		Vec3 _origin{ 0, 0, 0 };
		Vec3 _vector{ 0, 0, 0 };
	};
}
