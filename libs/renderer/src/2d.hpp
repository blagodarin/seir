// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_graphics/color.hpp>
#include <seir_math/vec.hpp>

namespace seir
{
	struct Vertex2D
	{
		seir::Vec2 _position;
		seir::Vec2 _texture;
		seir::Rgba32 _color;
	};
}
