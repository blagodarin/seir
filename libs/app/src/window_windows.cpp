// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "window_windows.hpp"

#include "app_windows.hpp"

namespace seir
{
	void HwndDeleter::free(HWND hwnd) noexcept
	{
		if (hwnd && !::DestroyWindow(hwnd))
			windows::reportError("DestroyWindow");
	}

	WindowsWindow::WindowsWindow(SharedPtr<WindowsApp>&& app, Hwnd&& hwnd) noexcept
		: _app{ std::move(app) }
		, _hwnd{ std::move(hwnd) }
	{
		_app->addWindow(_hwnd, this);
	}

	void WindowsWindow::close() noexcept
	{
		::SendMessageW(_hwnd, WM_CLOSE, 0, 0);
	}

	void WindowsWindow::show() noexcept
	{
		::ShowWindow(_hwnd, SW_SHOW);
		::UpdateWindow(_hwnd);
		::SetForegroundWindow(_hwnd);
		::SetFocus(_hwnd);
	}

	UniquePtr<Window> Window::create(const SharedPtr<App>& app)
	{
		auto windowsApp = staticCast<WindowsApp>(app);
		if (Hwnd hwnd{ ::CreateWindowExW(WS_EX_APPWINDOW, WindowsApp::kWindowClass, L"", WS_OVERLAPPEDWINDOW,
				0, 0, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, windowsApp->instance(), windowsApp.get()) })
			return makeUnique<Window, WindowsWindow>(std::move(windowsApp), std::move(hwnd));
		return {};
	}
}
