// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_app/events.hpp>
#include <seir_base/shared_ptr.hpp>
#include <seir_gui/style.hpp>

#include <vector>

namespace seir
{
	class Font;
	class GuiContext;
	class GuiFrame;
	class GuiLayout;
	class RectF;

	class GuiContextImpl final : public EventCallbacks
	{
	public:
		struct KeyCapture
		{
			unsigned pressed = 0;
			bool released = false;
		};

		explicit GuiContextImpl(Window&) noexcept;

		RectF addItem() const noexcept;
		KeyCapture captureClick(Key key, bool repeated, bool release = false) noexcept;

	private:
		void onKeyEvent(Window&, const KeyEvent&) override;
		void onTextEvent(Window&, std::string_view) override;

	private:
		Window& _window;
		SharedPtr<Font> _defaultFont;
		std::vector<uint16_t> _inputEvents;
		GuiLayout* _layout = nullptr;
		GuiLabelStyle _labelStyle;
		friend GuiContext;
		friend GuiFrame;
		friend GuiLayout;
	};
}
