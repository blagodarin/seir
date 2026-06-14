// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_ui/context.hpp>

#include <seir_ui/font.hpp>
#include "context_impl.hpp"

namespace seir
{
	UiContext::UiContext(Window& window)
		: UiContext{ window, {} } {}

	UiContext::UiContext(Window& window, const SharedPtr<Font>& defaultFont)
		: _impl{ std::make_unique<UiContextImpl>(window, defaultFont) } {}

	UiContext::~UiContext() noexcept = default;

	EventCallbacks& UiContext::eventCallbacks() noexcept
	{
		return *_impl;
	}

	void UiContext::setDefaultFont(const SharedPtr<Font>& font) noexcept
	{
		_impl->_defaultFont = font;
	}
}
