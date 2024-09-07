// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/shared_ptr.hpp>
#include <seir_graphics/color.hpp>

namespace seir
{
	class Font;

	class GuiButtonStyle
	{
	public:
		struct State
		{
			Rgba32 _backgroundColor;
			Rgba32 _textColor;
			constexpr State(Rgba32 backgroundColor, Rgba32 textColor) noexcept
				: _backgroundColor{ backgroundColor }, _textColor{ textColor } {}
		};
		SharedPtr<Font> _font;
		float _fontSize = 0;
		State _normal;
		State _hovered;
		State _pressed;
		GuiButtonStyle() noexcept;
	};

	class GuiEditStyle
	{
	public:
		struct State
		{
			Rgba32 _backgroundColor;
			Rgba32 _textColor;
			constexpr State(Rgba32 backgroundColor, Rgba32 textColor) noexcept
				: _backgroundColor{ backgroundColor }, _textColor{ textColor } {}
		};
		SharedPtr<Font> _font;
		float _fontSize = 0;
		State _normal;
		State _hovered;
		State _active;
		Rgba32 _cursorColor;
		Rgba32 _selectionColor;
		GuiEditStyle() noexcept;
	};

	class GuiLabelStyle
	{
	public:
		SharedPtr<Font> _font;
		float _fontSize = 0;
		Rgba32 _textColor;
		GuiLabelStyle() noexcept;
	};
}
