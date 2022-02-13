// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

namespace seir
{
	// Euler angles.
	class Euler
	{
	public:
		float _yaw = 0;   // Rotation around the vertical axis.
		float _pitch = 0; // Rotation around the horizontal axis.
		float _roll = 0;  //

		constexpr Euler() noexcept = default;
		constexpr Euler(float yaw, float pitch, float roll) noexcept
			: _yaw{ yaw }, _pitch{ pitch }, _roll{ roll } {}
	};
}
