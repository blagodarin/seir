// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "window_windows.hpp"

#include "app_windows.hpp"

#include <memory>

namespace
{
	std::unique_ptr<wchar_t[]> toWChar(const std::string& text)
	{
		if (!text.empty())
		{
			if (const auto wtextSize = ::MultiByteToWideChar(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), nullptr, 0))
			{
				auto wtext = std::make_unique<wchar_t[]>(wtextSize + 1u);
				if (::MultiByteToWideChar(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), wtext.get(), wtextSize))
				{
					wtext[static_cast<size_t>(wtextSize)] = L'\0';
					return wtext;
				}
			}
			seir::windows::reportError("MultiByteToWideChar");
		}
		return {};
	}
}

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

	WindowDescriptor WindowsWindow::descriptor() const noexcept
	{
		return { _app->instance(), reinterpret_cast<intptr_t>(_hwnd.get()) };
	}

	void WindowsWindow::show() noexcept
	{
		::ShowWindow(_hwnd, SW_SHOW);
		::UpdateWindow(_hwnd);
		::SetForegroundWindow(_hwnd);
		::SetFocus(_hwnd);
	}

	Size2D WindowsWindow::size() const noexcept
	{
		RECT clientRect{};
		::GetClientRect(_hwnd, &clientRect);
		return { clientRect.right - clientRect.left, clientRect.bottom - clientRect.top };
	}

	void WindowsWindow::reset() noexcept
	{
		_hwnd.reset();
	}

	UniquePtr<Window> Window::create(const SharedPtr<App>& app, const std::string& title)
	{
		const auto wtitle = ::toWChar(title);
		auto windowsApp = staticCast<WindowsApp>(app);
		if (Hwnd hwnd{ ::CreateWindowExW(WS_EX_APPWINDOW, WindowsApp::kWindowClass, wtitle ? wtitle.get() : L"", WS_OVERLAPPEDWINDOW,
				CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, windowsApp->instance(), windowsApp.get()) })
			return makeUnique<Window, WindowsWindow>(std::move(windowsApp), std::move(hwnd));
		return {};
	}
}
