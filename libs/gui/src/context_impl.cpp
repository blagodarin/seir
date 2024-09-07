// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "context_impl.hpp"

#include <seir_gui/font.hpp>
#include <seir_gui/layout.hpp>
#include <seir_renderer/renderer.hpp>

#include <cassert>
#include <algorithm>

namespace
{
	// Input event flags and masks.
	constexpr uint16_t kPayloadMask = 0x00ff;
	constexpr uint16_t kShiftFlag = 0x0100;
	constexpr uint16_t kPressedFlag = 0x1000;
	constexpr uint16_t kRepeatedFlag = 0x2000;
	constexpr uint16_t kTextFlag = 0x4000;
	constexpr uint16_t kProcessedFlag = 0x8000;
	constexpr uint16_t kKeySearchMask = kPayloadMask | kTextFlag | kProcessedFlag;
}

namespace seir
{
	GuiContextImpl::GuiContextImpl(Window& window) noexcept
		: _window{ window }
	{
	}

	GuiContextImpl::~GuiContextImpl() noexcept = default;

	RectF GuiContextImpl::addItem() const noexcept
	{
		return _layout ? _layout->addItem() : RectF{};
	}

	GuiContextImpl::KeyCapture GuiContextImpl::captureClick(Key key, bool repeated, bool release) noexcept
	{
		const auto i = std::find_if(_inputEvents.begin(), _inputEvents.end(), [key](const auto event) {
			return key == Key::None ? !(event & (kTextFlag | kProcessedFlag)) : (event & kKeySearchMask) == static_cast<uint8_t>(key);
		});
		if (i == _inputEvents.end())
			return { 0u, false };
		if (release && (*i & kPressedFlag))
			return { 0u, true };
		*i |= kProcessedFlag;
		if (!(*i & kPressedFlag))
			return { 0u, true };
		auto count = static_cast<unsigned>(!(*i & kRepeatedFlag) || repeated);
		for (auto j = std::next(i); j != _inputEvents.end(); ++j)
		{
			if ((*j & kKeySearchMask) == (*i & kPayloadMask))
			{
				if (!(*j & kRepeatedFlag))
				{
					if (!(*j & kPressedFlag))
						*j |= kProcessedFlag;
					return { count, true };
				}
				assert(*j & kPressedFlag);
				*j |= kProcessedFlag;
				if (repeated)
					++count;
			}
		}
		return { count, false };
	}

	void GuiContextImpl::captureKeyboard(std::function<bool(Key, bool)>&& keyCallback, std::function<void(std::string_view)>&& textCallback)
	{
		assert(!_keyboardItemId.empty());
		for (auto& event : _inputEvents)
		{
			if (event & kProcessedFlag)
				continue;
			event |= kProcessedFlag;
			if (event & kTextFlag)
				textCallback(_textInputs[event & size_t{ kPayloadMask }]);
			else if ((event & kPressedFlag) && !keyCallback(static_cast<Key>(event & kPayloadMask), event & kShiftFlag))
				break;
		}
	}

	std::optional<Vec2> GuiContextImpl::takeMouseCursor(const RectF& rect) noexcept
	{
		if (_mouseCursorTaken || !rect.contains(_mouseCursor))
			return {};
		_mouseCursorTaken = true;
		_mouseHoverTaken = true;
		return _mouseCursor;
	}

	std::optional<Vec2> GuiContextImpl::takeMouseHover(const RectF& rect) noexcept
	{
		if (_mouseHoverTaken || !rect.contains(_mouseCursor))
			return {};
		_mouseHoverTaken = true;
		return _mouseCursor;
	}

	void GuiContextImpl::updateWhiteTexture(const SharedPtr<Font>& font) noexcept
	{
		if (font)
		{
			_whiteTexture = font->bitmapTexture();
			_whiteTextureRect = font->whiteRect();
		}
		else
			_whiteTexture = {};
	}

	void GuiContextImpl::onKeyEvent([[maybe_unused]] Window& window, const KeyEvent& event)
	{
		assert(&window == &_window);
		auto encodedEvent = static_cast<uint16_t>(event._key);
		if (event._pressed)
		{
			encodedEvent |= kPressedFlag;
			if (event._repeated)
				encodedEvent |= kRepeatedFlag;
			if (event._shiftPressed)
				encodedEvent |= kShiftFlag;
		}
		_inputEvents.emplace_back(encodedEvent);
	}

	void GuiContextImpl::onTextEvent([[maybe_unused]] Window& window, std::string_view text)
	{
		assert(&window == &_window);
		const auto index = _textInputs.size();
		if (index >= kPayloadMask)
			return;
		_inputEvents.reserve(_inputEvents.size() + 1);
		_textInputs.emplace_back(text);
		_inputEvents.emplace_back(static_cast<uint16_t>(kTextFlag | index));
	}
}
