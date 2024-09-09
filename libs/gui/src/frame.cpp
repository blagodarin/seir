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
	constexpr seir::RectF relativeHeightInRect(const seir::RectF& rect, float relativeHeight) noexcept
	{
		const auto padding = rect.height() * (1 - relativeHeight) / 2;
		return { rect.topLeft() + padding, rect.bottomRight() - padding };
	}

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
		_context._keyboardItemPresent = false;
		_context._focusExpected = false;
		_context.updateWhiteTexture(_context._defaultFont);
		setButtonStyle({});
		setEditStyle({});
		setLabelStyle({});
	}

	GuiFrame::~GuiFrame() noexcept
	{
		// TODO: Release keyboard item focus on mouse click outside of the item.
		if (_context._mouseItemKey != Key::None && _context.captureClick(_context._mouseItemKey, false, true).released)
		{
			_context._mouseItemId.clear();
			_context._mouseItemKey = Key::None;
		}
		if (!_context._mouseItemPresent)
		{
			_context._mouseItemId.clear();
			_context._mouseItemKey = Key::None;
		}
		if (!_context._keyboardItemPresent)
			_context._keyboardItemId.clear();
		_context._inputEvents.clear();
		_context._textInputs.clear();
	}

	bool GuiFrame::addButton(std::string_view id, std::string_view text)
	{
		assert(!id.empty());
		const auto rect = _context.addItem();
		if (rect.isEmpty())
			return false;
		bool clicked = false;
		const auto* styleState = &_context._buttonStyle._normal;
		if (_context._mouseItemId == id)
		{
			assert(!_context._mouseHoverTaken);
			assert(!_context._mouseItemPresent);
			assert(_context._keyboardItemId.empty());
			const auto hovered = rect.contains(_context._mouseCursor);
			const auto released = _context.captureClick(_context._mouseItemKey, false, true).released;
			if (released)
			{
				_context._mouseItemId.clear();
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
		else if (_context._mouseItemId.empty() && _context.takeMouseHover(rect))
		{
			assert(!_context._mouseItemPresent);
			styleState = &_context._buttonStyle._hovered;
			if (const auto [pressed, released] = _context.captureClick(Key::Mouse1, false); pressed)
			{
				if (!released)
				{
					_context._mouseItemId = id;
					_context._mouseItemPresent = true;
					_context._mouseItemKey = Key::Mouse1;
					styleState = &_context._buttonStyle._pressed;
				}
				else
					clicked = true;
				_context._keyboardItemId.clear();
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

	std::optional<Vec2> GuiFrame::addDragArea(std::string_view id, const SizeF& size, Key key)
	{
		assert(!id.empty());
		const auto rect = _context.addItem(size);
		if (rect.isEmpty())
			return {};
		if (_context._mouseItemId == id)
		{
			assert(!_context._mouseHoverTaken);
			assert(!_context._mouseItemPresent);
			auto captured = rect.bound(_context._mouseCursor);
			_context._mouseHoverTaken = true;
			_context._mouseItemPresent = true;
			return captured;
		}
		if (_context._mouseItemId.empty())
		{
			assert(!_context._mouseItemPresent);
			if (auto maybeHover = _context.takeMouseHover(rect))
				if (const auto [pressed, released] = _context.captureClick(key, false); pressed)
				{
					if (!released)
					{
						_context._mouseItemId = id;
						_context._mouseItemPresent = true;
						_context._mouseItemKey = key;
					}
					_context._keyboardItemId.clear();
					return maybeHover;
				}
		}
		return {};
	}

	std::optional<Vec2> GuiFrame::addHoverArea(const SizeF& size) noexcept
	{
		return _context.takeMouseHover(_context.addItem(size));
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

	bool GuiFrame::addStringEdit(std::string_view id, std::string& text)
	{
		assert(!id.empty());
		const auto itemRect = _context.addItem();
		if (itemRect.isEmpty())
			return false;
		bool entered = false;
		const auto* styleState = &_context._editStyle._normal;
		bool active = false;
		if (_context._mouseItemId == id)
		{
			assert(!_context._mouseHoverTaken);
			assert(!_context._mouseItemPresent);
			assert(_context._keyboardItemId == id);
			if (_context.captureClick(_context._mouseItemKey, false, true).released)
			{
				_context._mouseItemId.clear();
				_context._mouseItemKey = Key::None;
				if (itemRect.contains(_context._mouseCursor))
					_context._mouseHoverTaken = true;
			}
			else
			{
				styleState = &_context._editStyle._active;
				_context._mouseHoverTaken = true;
			}
		}
		else if (_context._mouseItemId.empty() && _context.takeMouseHover(itemRect))
		{
			styleState = &_context._editStyle._hovered;
			if (const auto [pressed, released] = _context.captureClick(Key::Mouse1, false); pressed)
			{
				if (!released)
				{
					_context._mouseItemId = id;
					_context._mouseItemPresent = true;
					_context._mouseItemKey = Key::Mouse1;
				}
				_context._keyboardItemId = id;
				_context._keyboardItemPresent = false;
				_context._keyboardItem.setFocus();
			}
		}
		if (std::exchange(_context._focusExpected, false))
			if (_context._keyboardItemId.empty() && _context._mouseItemId.empty())
			{
				_context._keyboardItemId = id;
				_context._keyboardItemPresent = false;
				_context._keyboardItem.setFocus();
			}
		if (_context._keyboardItemId == id)
		{
			assert(!_context._keyboardItemPresent);
			_context._keyboardItemPresent = true;
			styleState = &_context._editStyle._active;
			active = true;
			_context._keyboardItem.adjustToText(text);
			_context.captureKeyboard(
				[&](Key key, bool shift) {
					switch (key)
					{
					case Key::Enter:
					case Key::NumEnter:
						entered = true;
						[[fallthrough]];
					case Key::Escape:
						_context._keyboardItemId.clear();
						active = false;
						return false;
					case Key::Left:
						_context._keyboardItem.onLeft(text, shift);
						break;
					case Key::Right:
						_context._keyboardItem.onRight(text, shift);
						break;
					case Key::Backspace:
						_context._keyboardItem.onBackspace(text);
						break;
					case Key::Delete:
						_context._keyboardItem.onDelete(text);
						break;
					case Key::Home:
						_context._keyboardItem.onHome(shift);
						break;
					case Key::End:
						_context._keyboardItem.onEnd(text, shift);
						break;
					default:
						break;
					}
					return true;
				},
				[&](std::string_view paste) {
					_context._keyboardItem.onPaste(text, paste);
				});
		}
		_context.updateWhiteTexture(_context._editStyle._font);
		selectWhiteTexture();
		_renderer.setColor(styleState->_backgroundColor);
		_renderer.addRect(itemRect);
		if (_context._editStyle._font)
		{
			const auto textRect = ::relativeHeightInRect(itemRect, _context._editStyle._fontSize);
			auto capture = _context._keyboardItem.fontCapture();
			_context._editStyle._font->textWidth(text, textRect.height(), &capture);
			if (active && capture._selectionRange)
			{
				const auto selectionLeft = textRect.left() + capture._selectionRange->first;
				if (selectionLeft < textRect.right())
				{
					const auto selectionRight = std::min(textRect.left() + capture._selectionRange->second, textRect.right());
					_renderer.setColor(_context._editStyle._selectionColor);
					_renderer.addRect({ { selectionLeft, textRect.top() }, seir::Vec2{ selectionRight, textRect.bottom() } });
				}
			}
			_renderer.setColor(styleState->_textColor);
			_context._editStyle._font->renderLine(_renderer, textRect, text);
			if (active && capture._cursorPosition)
			{
				const auto cursorX = textRect.left() + *capture._cursorPosition;
				if (cursorX < textRect.right() && _context._keyboardItem.isCursorPhaseVisible())
				{
					_renderer.setTextureRect(_context._editStyle._font->whiteRect());
					_renderer.setColor(_context._editStyle._cursorColor);
					_renderer.addRect({ { cursorX, textRect.top() }, seir::Vec2{ std::min(cursorX + 2, textRect.right()), textRect.bottom() } });
				}
			}
		}
		return entered;
	}

	void GuiFrame::close() noexcept
	{
		_context._window.close();
	}

	void GuiFrame::putKeyboardFocus() noexcept
	{
		if (_context._keyboardItemId.empty())
			_context._focusExpected = true;
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

	void GuiFrame::setEditStyle(const GuiEditStyle& style) noexcept
	{
		_context._editStyle = style;
		if (!_context._editStyle._font)
			_context._editStyle._font = _context._defaultFont;
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

	std::optional<Vec2> GuiFrame::takeMouseCursor() noexcept
	{
		return _context.takeMouseCursor(RectF{ _size });
	}
}
