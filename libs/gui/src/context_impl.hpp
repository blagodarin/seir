// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_app/events.hpp>
#include <seir_base/shared_ptr.hpp>
#include <seir_graphics/rectf.hpp>
#include <seir_gui/style.hpp>
#include "keyboard_item.hpp"

#include <functional>
#include <optional>
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
		void captureKeyboard(std::function<bool(Key, bool)>&& keyCallback, std::function<void(std::string_view)>&& textCallback);
		std::optional<Vec2> takeMouseCursor(const RectF&) noexcept;
		std::optional<Vec2> takeMouseHover(const RectF&) noexcept;
		void updateWhiteTexture(const SharedPtr<Font>&) noexcept;

	private:
		void onKeyEvent(Window&, const KeyEvent&) override;
		void onTextEvent(Window&, std::string_view) override;

	private:
		Window& _window;
		std::vector<uint16_t> _inputEvents;
		std::vector<std::string> _textInputs;
		Vec2 _mouseCursor;
		bool _mouseCursorTaken = false;
		bool _mouseHoverTaken = false;
		std::string _mouseItemId;
		bool _mouseItemPresent = false;
		Key _mouseItemKey = Key::None;
		std::string _keyboardItemId;
		bool _keyboardItemPresent = false;
		GuiKeyboardItem _keyboardItem;
		GuiButtonStyle _buttonStyle;
		GuiEditStyle _editStyle;
		GuiLabelStyle _labelStyle;
		SharedPtr<Font> _defaultFont;
		SharedPtr<Texture2D> _whiteTexture;
		RectF _whiteTextureRect;
		GuiLayout* _layout = nullptr;
		bool _focusExpected = false;
		friend GuiContext;
		friend GuiFrame;
		friend GuiLayout;
	};
}
