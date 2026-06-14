// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_graphics/color.hpp>
#include <seir_ui/font.hpp>

namespace seir
{
	class UiButtonStyle
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

		UiButtonStyle() noexcept;
	};

	class UiEditStyle
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

		UiEditStyle() noexcept;
	};

	class UiLabelStyle
	{
	public:
		SharedPtr<Font> _font;
		float _fontSize = 0;
		Rgba32 _textColor;

		UiLabelStyle() noexcept;
		constexpr UiLabelStyle(Rgba32 textColor, float fontSize) noexcept
			: _fontSize{ fontSize }, _textColor{ textColor } {}
	};
}
