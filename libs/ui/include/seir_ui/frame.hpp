// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_app/key.hpp>
#include <seir_math/size.hpp>

#include <optional>
#include <string>

namespace seir
{
	class Canvas;
	class Font;
	class UiButtonStyle;
	class UiContext;
	class UiEditStyle;
	class UiLabelStyle;
	class Vec2;

	enum class UiAlignment
	{
		Left,
		Center,
		Right,
	};

	class UiFrame
	{
	public:
		UiFrame(UiContext&, Canvas&);
		~UiFrame() noexcept;

		bool addButton(std::string_view id, std::string_view text);
		std::optional<Vec2> addDragArea(std::string_view id, const Size2D&, Key);
		std::optional<Vec2> addHoverArea(const Size2D&) noexcept;
		void addLabel(std::string_view text, UiAlignment = UiAlignment::Left);
		bool addStringEdit(std::string_view id, std::string& text);
		void close() noexcept;
		void putKeyboardFocus() noexcept;
		[[nodiscard]] Canvas& canvas() noexcept { return _canvas; }
		void selectWhiteTexture();
		void setButtonStyle(const UiButtonStyle&) noexcept;
		void setEditStyle(const UiEditStyle&) noexcept;
		void setLabelStyle(const UiLabelStyle&) noexcept;
		[[nodiscard]] Size2D size() const noexcept { return _size; }
		bool takeAnyKeyPress() noexcept;
		Vec2 takeBorderHover(float borderWidth) noexcept;
		bool takeKeyDown(Key) noexcept;
		bool takeKeyPress(Key) noexcept;
		std::optional<bool> takeKeyState(Key) noexcept;
		std::optional<Vec2> takeMouseCursor() noexcept;

	private:
		class UiContextImpl& _context;
		Canvas& _canvas;
		const Size2D _size;
		friend class UiLayout;
	};
}
