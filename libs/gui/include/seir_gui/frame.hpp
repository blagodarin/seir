// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_app/key.hpp>
#include <seir_graphics/sizef.hpp>

#include <optional>
#include <string>

namespace seir
{
	class Font;
	class GuiButtonStyle;
	class GuiContext;
	class GuiEditStyle;
	class GuiLabelStyle;
	class Renderer2D;
	class Vec2;

	enum class GuiAlignment
	{
		Left,
		Center,
		Right,
	};

	class GuiFrame
	{
	public:
		GuiFrame(GuiContext&, Renderer2D&);
		~GuiFrame() noexcept;

		bool addButton(std::string_view id, std::string_view text);
		std::optional<Vec2> addDragArea(std::string_view id, const SizeF&, Key);
		std::optional<Vec2> addHoverArea(const SizeF&) noexcept;
		void addLabel(std::string_view text, GuiAlignment = GuiAlignment::Left);
		bool addStringEdit(std::string_view id, std::string& text);
		void close() noexcept;
		void putKeyboardFocus() noexcept;
		Renderer2D& renderer() noexcept { return _renderer; }
		void selectWhiteTexture();
		void setButtonStyle(const GuiButtonStyle&) noexcept;
		void setEditStyle(const GuiEditStyle&) noexcept;
		void setLabelStyle(const GuiLabelStyle&) noexcept;
		[[nodiscard]] SizeF size() const noexcept { return _size; }
		bool takeAnyKeyPress() noexcept;
		bool takeKeyPress(Key) noexcept;
		std::optional<bool> takeKeyState(Key) noexcept;
		std::optional<Vec2> takeMouseCursor() noexcept;

	private:
		class GuiContextImpl& _context;
		Renderer2D& _renderer;
		const SizeF _size;
		friend class GuiLayout;
	};
}
