// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_app/key.hpp>
#include <seir_graphics/sizef.hpp>

#include <string_view>

namespace seir
{
	class Font;
	class GuiContext;
	class GuiLabelStyle;
	class Renderer2D;

	enum class GuiAlignment
	{
		Left,
		Center,
		Right,
	};

	class GuiFrame
	{
	public:
		explicit GuiFrame(GuiContext&, Renderer2D&);
		~GuiFrame() noexcept;

		void addLabel(std::string_view text, GuiAlignment = GuiAlignment::Left);
		void setLabelStyle(const GuiLabelStyle&) noexcept;
		bool takeAnyKeyPress() noexcept;
		bool takeKeyPress(Key) noexcept;

	private:
		class GuiContextImpl& _context;
		Renderer2D& _renderer;
		const SizeF _size;
		friend class GuiLayout;
	};
}
