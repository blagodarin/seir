// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <memory>

namespace seir
{
	class EventCallbacks;
	class Window;

	class GuiContext
	{
	public:
		explicit GuiContext(Window&);
		~GuiContext() noexcept;

		EventCallbacks& eventCallbacks() noexcept;

	private:
		const std::unique_ptr<class GuiContextImpl> _impl;
		friend class GuiFrame;
	};
}
