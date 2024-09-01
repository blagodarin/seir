// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/shared_ptr.hpp>
#include <seir_graphics/color.hpp>

namespace seir
{
	class Font;

	class GuiLabelStyle
	{
	public:
		SharedPtr<Font> _font;
		float _fontSize = 0;
		Rgba32 _textColor;
		GuiLabelStyle() noexcept;
	};
}
