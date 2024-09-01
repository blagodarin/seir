// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_gui/frame.hpp>

#include <seir_app/window.hpp>
#include <seir_gui/context.hpp>
#include "context_impl.hpp"

namespace seir
{
	GuiFrame::GuiFrame(GuiContext& context)
		: _context{ *context._impl }
		, _size{ context._impl->_window.size() }
	{
	}

	GuiFrame::~GuiFrame() noexcept
	{
		_context._inputEvents.clear();
	}

	bool GuiFrame::takeAnyKeyPress() noexcept
	{
		return takeKeyPress(Key::None);
	}

	bool GuiFrame::takeKeyPress(Key key) noexcept
	{
		return _context.captureClick(key, false).pressed > 0;
	}
}
