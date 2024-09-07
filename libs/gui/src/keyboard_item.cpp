// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "keyboard_item.hpp"

#include <utility>

namespace seir
{
	void GuiKeyboardItem::onBackspace(std::string& text) noexcept
	{
		assert(_cursor <= text.size());
		size_t count = 0;
		if (_selectionSize > 0)
		{
			assert(_cursor == _selectionOffset
				|| _cursor == _selectionOffset + _selectionSize);
			count = std::exchange(_selectionSize, 0);
			_cursor = _selectionOffset;
		}
		else if (_cursor > 0)
		{
			count = leftStep(text);
			_cursor -= count;
		}
		else
			return;
		text.erase(_cursor, count);
		_cursorMark = std::chrono::steady_clock::now();
	}

	void GuiKeyboardItem::onDelete(std::string& text) noexcept
	{
		assert(_cursor <= text.size());
		size_t count = 0;
		if (_selectionSize > 0)
		{
			assert(_cursor == _selectionOffset
				|| _cursor == _selectionOffset + _selectionSize);
			count = std::exchange(_selectionSize, 0);
			_cursor = _selectionOffset;
		}
		else if (_cursor < text.size())
			count = rightStep(text);
		else
			return;
		text.erase(_cursor, count);
		_cursorMark = std::chrono::steady_clock::now();
	}

	void GuiKeyboardItem::onEnd(std::string_view text, bool shift) noexcept
	{
		if (_cursor < text.size())
		{
			if (shift)
			{
				if (!_selectionSize)
					_selectionOffset = _cursor;
				else if (_cursor == _selectionOffset)
					_selectionOffset += _selectionSize;
				_selectionSize = text.size() - _selectionOffset;
			}
			_cursor = text.size();
			_cursorMark = std::chrono::steady_clock::now();
		}
		if (!shift)
			_selectionSize = 0;
	}

	void GuiKeyboardItem::onHome(bool shift) noexcept
	{
		if (_cursor > 0)
		{
			if (shift)
			{
				if (_selectionSize > 0 && _selectionOffset < _cursor)
					_selectionSize = _selectionOffset;
				else
					_selectionSize += _cursor;
				_selectionOffset = 0;
			}
			_cursor = 0;
			_cursorMark = std::chrono::steady_clock::now();
		}
		if (!shift)
			_selectionSize = 0;
	}

	void GuiKeyboardItem::onLeft(std::string_view text, bool shift) noexcept
	{
		if (_cursor > 0)
		{
			const auto step = leftStep(text);
			assert(step > 0 && step <= _cursor);
			_cursor -= step;
			_cursorMark = std::chrono::steady_clock::now();
			if (shift)
			{
				if (_selectionSize > 0 && _selectionOffset <= _cursor)
					_selectionSize -= step;
				else
				{
					_selectionSize += step;
					_selectionOffset = _cursor;
				}
			}
		}
		if (!shift)
			_selectionSize = 0;
	}

	void GuiKeyboardItem::onPaste(std::string& text, std::string_view paste)
	{
		if (_selectionSize > 0)
		{
			text.erase(_selectionOffset, _selectionSize);
			_cursor = _selectionOffset;
			_selectionSize = 0;
		}
		text.insert(_cursor, paste);
		_cursor += paste.size();
		_cursorMark = std::chrono::steady_clock::now();
	}

	void GuiKeyboardItem::onRight(std::string_view text, bool shift) noexcept
	{
		if (_cursor < text.size())
		{
			const auto step = rightStep(text);
			assert(step > 0 && step <= text.size() - _cursor);
			if (shift)
			{
				if (_selectionSize > 0 && _selectionOffset == _cursor)
				{
					_selectionSize -= step;
					_selectionOffset += step;
				}
				else
				{
					_selectionOffset = _cursor - _selectionSize;
					_selectionSize += step;
				}
			}
			_cursor += step;
			_cursorMark = std::chrono::steady_clock::now();
		}
		if (!shift)
			_selectionSize = 0;
	}

	void GuiKeyboardItem::setFocus(std::string_view id)
	{
		_id = id;
		_present = false;
		_cursor = std::numeric_limits<size_t>::max();
		_cursorMark = std::chrono::steady_clock::now();
		_selectionOffset = 0;
		_selectionSize = _cursor;
	}
}
