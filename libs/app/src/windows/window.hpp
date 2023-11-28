// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_app/window.hpp>

#include "helpers.hpp"

namespace seir
{
	class WindowsApp;

	class WindowsWindow final : public Window
	{
	public:
		WindowsWindow(SharedPtr<WindowsApp>&&, Hwnd&&) noexcept;

		void close() noexcept override;
		std::optional<Point> cursor() const noexcept override;
		WindowDescriptor descriptor() const noexcept override;
		void setIcon(const Image&) noexcept override;
		void setTitle(const std::string&) noexcept override;
		void show() noexcept override;
		Size size() const noexcept override;

		void reset() noexcept;

	private:
		const SharedPtr<WindowsApp> _app;
		Hicon _icon;
		Hwnd _hwnd;
	};
}
