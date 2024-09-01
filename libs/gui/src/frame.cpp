// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_gui/frame.hpp>

#include <seir_app/window.hpp>
#include <seir_graphics/color.hpp>
#include <seir_graphics/rectf.hpp>
#include <seir_gui/context.hpp>
#include <seir_gui/font.hpp>
#include <seir_renderer/2d.hpp>
#include "context_impl.hpp"

namespace seir
{
	GuiFrame::GuiFrame(GuiContext& context, Renderer2D& renderer)
		: _context{ *context._impl }
		, _renderer{ renderer }
		, _size{ context._impl->_window.size() }
	{
	}

	GuiFrame::~GuiFrame() noexcept
	{
		_context._inputEvents.clear();
	}

	void GuiFrame::addLabel(const Font& font, std::string_view text, GuiAlignment alignment)
	{
		auto textRect = _context.addItem();
		if (textRect.top() >= textRect.bottom())
			return;
		constexpr float relativeTextSize = 1.f;
		const auto verticalPadding = textRect.height() * (1 - relativeTextSize) / 2;
		textRect._top += verticalPadding;
		textRect._bottom -= verticalPadding;
		if (textRect.left() == textRect.right())
		{
			if (alignment == GuiAlignment::Left || alignment == GuiAlignment::Center)
				textRect._right = _size._width;
			if (alignment == GuiAlignment::Center || alignment == GuiAlignment::Right)
				textRect._left = 0;
		}
		if (const auto textWidth = font.textWidth(text, textRect.height()); textWidth < textRect.width())
		{
			if (alignment == GuiAlignment::Center)
			{
				const auto horizontalPadding = (textRect.width() - textWidth) / 2;
				textRect._left += horizontalPadding;
				textRect._right -= horizontalPadding;
			}
			else if (alignment == GuiAlignment::Right)
				textRect._left = textRect._right - textWidth;
		}
		_renderer.setColor(Rgba32::yellow());
		font.renderLine(_renderer, textRect, text);
		// TODO: Update white texture.
	}

	bool GuiFrame::takeAnyKeyPress() noexcept
	{
		return takeKeyPress(Key::None);
	}

	bool GuiFrame::takeKeyPress(Key key) noexcept
	{
		return _context.captureClick(key, false).pressed > 0;
	}
}
