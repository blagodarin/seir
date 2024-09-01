// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_gui/style.hpp>

#include <seir_gui/font.hpp>

namespace seir
{
	GuiLabelStyle::GuiLabelStyle() noexcept
		: _fontSize{ 1.f }
		, _textColor{ Rgba32::white() }
	{
	}
}
