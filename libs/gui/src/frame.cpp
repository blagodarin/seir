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

#include <cassert>

namespace
{
	constexpr seir::RectF sizeInRect(const seir::RectF& rect, const seir::SizeF& size) noexcept
	{
		const auto yPadding = (rect.height() - size._height) / 2;
		const auto xPadding = std::max(yPadding, (rect.width() - size._width) / 2);
		return { rect.topLeft() + seir::Vec2{ xPadding, yPadding }, rect.bottomRight() - seir::Vec2{ xPadding, yPadding } };
	}
}

namespace seir
{
	GuiFrame::GuiFrame(GuiContext& context, Renderer2D& renderer)
		: _context{ *context._impl }
		, _renderer{ renderer }
		, _size{ context._impl->_window.size() }
	{
		const auto cursor = _context._window.cursor().value_or(Point{ -1, -1 });
		_context._mouseCursor = { static_cast<float>(cursor._x), static_cast<float>(cursor._y) };
		_context._mouseCursorTaken = false;
		_context._mouseHoverTaken = false;
		_context._mouseItemPresent = false;
		// TODO: _context._keyboardItem._present = false;
		_context.updateWhiteTexture(_context._defaultFont);
		setButtonStyle({});
		setLabelStyle({});
	}

	GuiFrame::~GuiFrame() noexcept
	{
		if (_context._mouseItemKey != Key::None && _context.captureClick(_context._mouseItemKey, false, true).released)
		{
			_context._mouseItem.clear();
			_context._mouseItemKey = Key::None;
		}
		if (!_context._mouseItemPresent)
		{
			_context._mouseItem.clear();
			_context._mouseItemKey = Key::None;
		}
		_context._inputEvents.clear();
	}

	bool GuiFrame::addButton(std::string_view id, std::string_view text)
	{
		assert(!id.empty());
		const auto rect = _context.addItem();
		if (rect.isEmpty())
			return false;
		bool clicked = false;
		const auto* styleState = &_context._buttonStyle._normal;
		if (_context._mouseItem == id)
		{
			assert(!_context._mouseHoverTaken);
			assert(!_context._mouseItemPresent);
			// TODO: assert(_context._keyboardItem._id.empty());
			const auto hovered = rect.contains(_context._mouseCursor);
			const auto released = _context.captureClick(_context._mouseItemKey, false, true).released;
			if (released)
			{
				_context._mouseItem.clear();
				_context._mouseItemKey = Key::None;
				if (hovered)
				{
					clicked = true;
					styleState = &_context._buttonStyle._hovered;
					_context._mouseHoverTaken = true;
				}
			}
			else
			{
				styleState = &_context._buttonStyle._pressed;
				_context._mouseHoverTaken = true;
				_context._mouseItemPresent = true;
			}
		}
		else if (_context._mouseItem.empty() && _context.takeMouseHover(rect))
		{
			assert(!_context._mouseItemPresent);
			styleState = &_context._buttonStyle._hovered;
			if (const auto [pressed, released] = _context.captureClick(Key::Mouse1, false); pressed)
			{
				if (!released)
				{
					_context._mouseItem = id;
					_context._mouseItemPresent = true;
					_context._mouseItemKey = Key::Mouse1;
					styleState = &_context._buttonStyle._pressed;
				}
				else
					clicked = true;
				// TODO: _context._keyboardItem._id.clear();
			}
		}
		_context.updateWhiteTexture(_context._buttonStyle._font);
		selectWhiteTexture();
		_renderer.setColor(styleState->_backgroundColor);
		_renderer.addRect(rect);
		if (_context._buttonStyle._font)
		{
			const auto textHeight = rect.height() * _context._buttonStyle._fontSize;
			const auto textWidth = _context._buttonStyle._font->textWidth(text, textHeight);
			_renderer.setColor(styleState->_textColor);
			_context._buttonStyle._font->renderLine(_renderer, ::sizeInRect(rect, { textWidth, textHeight }), text);
		}
		return clicked;
	}

	void GuiFrame::addLabel(std::string_view text, GuiAlignment alignment)
	{
		if (!_context._labelStyle._font)
			return;
		auto textRect = _context.addItem();
		if (textRect.top() >= textRect.bottom())
			return;
		const auto verticalPadding = textRect.height() * (1 - _context._labelStyle._fontSize) / 2;
		textRect._top += verticalPadding;
		textRect._bottom -= verticalPadding;
		if (textRect.left() == textRect.right())
		{
			if (alignment == GuiAlignment::Left || alignment == GuiAlignment::Center)
				textRect._right = _size._width;
			if (alignment == GuiAlignment::Center || alignment == GuiAlignment::Right)
				textRect._left = 0;
		}
		if (const auto textWidth = _context._labelStyle._font->textWidth(text, textRect.height()); textWidth < textRect.width())
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
		_renderer.setColor(_context._labelStyle._textColor);
		_context._labelStyle._font->renderLine(_renderer, textRect, text);
		_context.updateWhiteTexture(_context._labelStyle._font);
	}

	void GuiFrame::selectWhiteTexture()
	{
		_renderer.setTexture(_context._whiteTexture);
		if (_context._whiteTexture)
			_renderer.setTextureRect(_context._whiteTextureRect);
	}

	void GuiFrame::setButtonStyle(const GuiButtonStyle& style) noexcept
	{
		_context._buttonStyle = style;
		if (!_context._buttonStyle._font)
			_context._buttonStyle._font = _context._defaultFont;
	}

	void GuiFrame::setLabelStyle(const GuiLabelStyle& style) noexcept
	{
		_context._labelStyle = style;
		if (!_context._labelStyle._font)
			_context._labelStyle._font = _context._defaultFont;
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
