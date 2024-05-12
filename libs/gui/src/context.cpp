// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_gui/context.hpp>

#include "context_impl.hpp"

namespace seir
{
	GuiContext::GuiContext(Window& window)
		: _impl{ std::make_unique<GuiContextImpl>(window) }
	{
	}

	GuiContext::~GuiContext() noexcept = default;

	EventCallbacks& GuiContext::eventCallbacks() noexcept
	{
		return *_impl;
	}
}
