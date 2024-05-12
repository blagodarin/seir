// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_app/window.hpp>

#include "helpers.hpp"

namespace seir
{
	class AppImpl;

	class WindowImpl
	{
	public:
		WindowImpl(AppImpl&, Window&, Hwnd&&) noexcept;

		void reset() noexcept;
		Window& window() noexcept { return _window; }

	private:
		static std::unique_ptr<WindowImpl> create(AppImpl&, Window&, const std::string&);

	private:
		const AppImpl& _app;
		Window& _window;
		Hicon _icon;
		Hwnd _hwnd;
		friend Window;
	};
}
