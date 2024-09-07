// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/utf8.hpp>

#include <cassert>
#include <chrono>
#include <string>

namespace seir
{
	struct GuiKeyboardItem
	{
		std::string _id;
		bool _present = false;
		size_t _cursor = 0;
		std::chrono::steady_clock::time_point _cursorMark;
		size_t _selectionOffset = 0;
		size_t _selectionSize = 0;

		constexpr void adjustToText(std::string_view text) noexcept
		{
			if (_cursor > text.size())
				_cursor = text.size();
			else if (_cursor < text.size())
				while (_cursor > 0 && seir::isUtf8Continuation(text[_cursor]))
					--_cursor;
			if (_selectionOffset > _cursor)
				_selectionOffset = _cursor;
			if (const auto maxSelectionSize = text.size() - _selectionOffset; maxSelectionSize < _selectionSize)
				_selectionSize = maxSelectionSize;
		}

		void onBackspace(std::string& text) noexcept;
		void onDelete(std::string& text) noexcept;
		void onEnd(std::string_view text, bool shift) noexcept;
		void onHome(bool shift) noexcept;
		void onLeft(std::string_view text, bool shift) noexcept;
		void onPaste(std::string& text, std::string_view paste);
		void onRight(std::string_view text, bool shift) noexcept;
		void setFocus(std::string_view id);

	private:
		constexpr size_t leftStep(std::string_view text) const noexcept
		{
			assert(_cursor > 0);
			auto offset = _cursor;
			do
				--offset;
			while (offset > 0 && seir::isUtf8Continuation(text[offset]));
			return _cursor - offset;
		}

		constexpr size_t rightStep(std::string_view text) const noexcept
		{
			assert(_cursor < text.size());
			auto offset = _cursor;
			do
				++offset;
			while (offset < text.size() && seir::isUtf8Continuation(text[offset]));
			return offset - _cursor;
		}
	};
}
