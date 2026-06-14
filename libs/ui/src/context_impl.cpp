// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "context_impl.hpp"

#include <seir_renderer/renderer.hpp>
#include <seir_ui/font.hpp>
#include <seir_ui/layout.hpp>

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
	UiContextImpl::UiContextImpl(Window& window, const SharedPtr<Font>& defaultFont) noexcept
		: _window{ window }
		, _defaultFont{ defaultFont }
	{
	}

	UiContextImpl::~UiContextImpl() noexcept = default;

	RectF UiContextImpl::addItem() const noexcept
	{
		return _layout ? _layout->addItem() : RectF{};
	}

	RectF UiContextImpl::addItem(const SizeF& size) const noexcept
	{
		return _layout ? _layout->addItem(size) : RectF{};
	}

	UiContextImpl::KeyCapture UiContextImpl::captureClick(Key key, bool repeated, bool release) noexcept
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

	void UiContextImpl::captureKeyboard(std::function<bool(Key, bool)>&& keyCallback, std::function<void(std::string_view)>&& textCallback)
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

	Vec2 UiContextImpl::takeBorderHover(const RectF& rect, float borderWidth) noexcept
	{
		if (_mouseHoverTaken)
			return {};

		const auto match = [borderWidth](float value, float min, float max) {
			if (value >= min)
			{
				if (value <= min + borderWidth)
					return -1.f;
				else if (value >= max - borderWidth && value <= max)
					return 1.f;
			}
			return 0.f;
		};

		const auto x = match(_mouseCursor.x, rect.left(), rect.right());
		const auto y = match(_mouseCursor.y, rect.top(), rect.bottom());
		_mouseHoverTaken = x != 0 || y != 0;
		return { x, y };
	}

	std::optional<Vec2> UiContextImpl::takeMouseCursor(const RectF& rect) noexcept
	{
		if (_mouseCursorTaken || !rect.contains(_mouseCursor))
			return {};
		_mouseCursorTaken = true;
		_mouseHoverTaken = true;
		return _mouseCursor;
	}

	std::optional<Vec2> UiContextImpl::takeMouseHover(const RectF& rect) noexcept
	{
		if (_mouseHoverTaken || !rect.contains(_mouseCursor))
			return {};
		_mouseHoverTaken = true;
		return _mouseCursor;
	}

	void UiContextImpl::updateWhiteTexture(const SharedPtr<Font>& font) noexcept
	{
		if (font)
		{
			_whiteTexture = font->bitmapTexture();
			_whiteTextureRect = font->whiteRect();
		}
		else
			_whiteTexture = {};
	}

	void UiContextImpl::onKeyEvent([[maybe_unused]] Window& window, const KeyEvent& event)
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
		_keyStates.update(event);
	}

	void UiContextImpl::onTextEvent([[maybe_unused]] Window& window, std::string_view text)
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
