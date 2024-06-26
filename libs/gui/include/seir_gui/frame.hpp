// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_app/key.hpp>

namespace seir
{
	class GuiContext;

	class GuiFrame
	{
	public:
		explicit GuiFrame(GuiContext&);
		~GuiFrame() noexcept;

		bool takeAnyKeyPress() noexcept;
		bool takeKeyPress(Key) noexcept;

	private:
		class GuiContextImpl& _context;
	};
}
