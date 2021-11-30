// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

namespace seir
{
	struct AudioFrame
	{
		float _left;
		float _right;

		constexpr void operator+=(const AudioFrame& other) noexcept
		{
			_left += other._left;
			_right += other._right;
		}
	};
}
