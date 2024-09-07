// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <chrono>
#include <string>

namespace seir
{
	class FontCapture;

	class GuiKeyboardItem
	{
	public:
		void adjustToText(std::string_view text) noexcept;
		FontCapture fontCapture() const noexcept;
		bool isCursorPhaseVisible() const noexcept;
		void onBackspace(std::string& text) noexcept;
		void onDelete(std::string& text) noexcept;
		void onEnd(std::string_view text, bool shift) noexcept;
		void onHome(bool shift) noexcept;
		void onLeft(std::string_view text, bool shift) noexcept;
		void onPaste(std::string& text, std::string_view paste);
		void onRight(std::string_view text, bool shift) noexcept;
		void setFocus() noexcept;

	private:
		size_t _cursor = 0;
		std::chrono::steady_clock::time_point _cursorMark;
		size_t _selectionOffset = 0;
		size_t _selectionSize = 0;
	};
}
