// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_graphics/color.hpp>
#include <seir_math/vec.hpp>

namespace seir
{
	struct CanvasVertex
	{
		Vec2 _position;
		Vec2 _texture;
		Rgba32 _color;
	};
}
