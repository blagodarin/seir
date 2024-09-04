// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_gui/style.hpp>

#include <seir_gui/font.hpp>

namespace seir
{
	GuiButtonStyle::GuiButtonStyle() noexcept
		: _fontSize{ .75f }
		, _normal{ Rgba32::grayscale(0x88, 0xcc), Rgba32::black() }
		, _hovered{ Rgba32::grayscale(0xdd, 0xdd), Rgba32::grayscale(0x11) }
		, _pressed{ Rgba32::white(0xee), Rgba32::grayscale(0x22) }
	{
	}

	GuiLabelStyle::GuiLabelStyle() noexcept
		: _fontSize{ 1.f }
		, _textColor{ Rgba32::white() }
	{
	}
}
