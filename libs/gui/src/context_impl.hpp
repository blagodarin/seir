// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_app/events.hpp>
#include <seir_base/shared_ptr.hpp>
#include <seir_graphics/rectf.hpp>
#include <seir_gui/style.hpp>

#include <optional>
#include <string>
#include <vector>

namespace seir
{
	class Font;
	class GuiContext;
	class GuiFrame;
	class GuiLayout;
	class RectF;
	class Texture2D;

	class GuiContextImpl final : public EventCallbacks
	{
	public:
		struct KeyCapture
		{
			unsigned pressed = 0;
			bool released = false;
		};

		explicit GuiContextImpl(Window&) noexcept;
		~GuiContextImpl() noexcept override;

		RectF addItem() const noexcept;
		KeyCapture captureClick(Key key, bool repeated, bool release = false) noexcept;
		std::optional<Vec2> takeMouseHover(const RectF&) noexcept;
		void updateWhiteTexture(const SharedPtr<Font>&) noexcept;

	private:
		void onKeyEvent(Window&, const KeyEvent&) override;
		void onTextEvent(Window&, std::string_view) override;

	private:
		Window& _window;
		SharedPtr<Font> _defaultFont;
		SharedPtr<Texture2D> _whiteTexture;
		RectF _whiteTextureRect;
		std::vector<uint16_t> _inputEvents;
		Vec2 _mouseCursor;
		bool _mouseCursorTaken = false;
		bool _mouseHoverTaken = false;
		std::string _mouseItem;
		bool _mouseItemPresent = false;
		Key _mouseItemKey = Key::None;
		GuiLayout* _layout = nullptr;
		GuiButtonStyle _buttonStyle;
		GuiLabelStyle _labelStyle;
		friend GuiContext;
		friend GuiFrame;
		friend GuiLayout;
	};
}
