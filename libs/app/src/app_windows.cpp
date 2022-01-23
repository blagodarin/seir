// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "app_windows.hpp"

#include <seir_base/unique_ptr.hpp>

#include <memory>

namespace
{
	seir::WindowsCursor createEmptyCursor(HINSTANCE instance)
	{
		const auto width = ::GetSystemMetrics(SM_CXCURSOR);
		const auto height = ::GetSystemMetrics(SM_CYCURSOR);
		const auto maskSize = static_cast<size_t>(width * height / 8);
		const auto buffer = std::make_unique<uint8_t[]>(2 * maskSize);
		std::memset(buffer.get(), 0xff, maskSize);            // AND mask;
		std::memset(buffer.get() + maskSize, 0x00, maskSize); // XOR mask.
		if (const auto cursor = ::CreateCursor(instance, 0, 0, width, height, buffer.get(), buffer.get() + maskSize))
			return seir::WindowsCursor{ cursor };
		seir::windows::reportError("CreateCursor");
		return {};
	}

	const wchar_t* const kWindowClass = L"Seir";

	bool registerWindowClass(HINSTANCE instance, HCURSOR cursor) noexcept
	{
		WNDCLASSEXW windowClass{ sizeof windowClass };
		windowClass.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
		windowClass.lpfnWndProc = seir::WindowsApp::staticWindowProc;
		windowClass.hInstance = instance;
		windowClass.hIcon = ::LoadIconA(nullptr, IDI_APPLICATION);
		windowClass.hCursor = cursor;
		windowClass.hbrBackground = static_cast<HBRUSH>(::GetStockObject(BLACK_BRUSH));
		windowClass.lpszClassName = kWindowClass;
		if (::RegisterClassExW(&windowClass))
			return true;
		seir::windows::reportError("RegisterClassExW");
		return false;
	}

	void unregisterWindowClass(HINSTANCE instance) noexcept
	{
		if (!::UnregisterClassW(kWindowClass, instance))
			seir::windows::reportError("UnregisterClassW");
	}
}

namespace seir
{
	void WindowsCursorDeleter::free(HCURSOR handle) noexcept
	{
		if (handle && !::DestroyCursor(handle))
			windows::reportError("DestroyCursor");
	}

	WindowsApp::WindowsApp(HINSTANCE instance, WindowsCursor&& emptyCursor)
		: _instance{ instance }
		, _emptyCursor{ std::move(emptyCursor) }
	{
	}

	WindowsApp::~WindowsApp() noexcept
	{
		::unregisterWindowClass(_instance);
	}

	bool WindowsApp::processEvents()
	{
		if (hasQuit())
			return false;
		MSG msg;
		while (::PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				endQuit();
				return false;
			}
			::TranslateMessage(&msg);
			::DispatchMessageW(&msg);
		}
		return true;
	}

	void WindowsApp::quit() noexcept
	{
		if (beginQuit())
			::PostQuitMessage(0);
	}

	LRESULT CALLBACK WindowsApp::staticWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) noexcept
	{
		WindowsApp* app = nullptr;
		if (msg == WM_NCCREATE)
		{
			app = static_cast<WindowsApp*>(((CREATESTRUCTW*)lparam)->lpCreateParams);
			::SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)app);
		}
		else
			app = (WindowsApp*)::GetWindowLongPtrW(hwnd, GWLP_USERDATA);
		return app->windowProc(hwnd, msg, wparam, lparam);
	}

	LRESULT WindowsApp::windowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) noexcept
	{
		return ::DefWindowProcW(hwnd, msg, wparam, lparam);
	}

	UniquePtr<App> App::create()
	{
		const auto instance = ::GetModuleHandleW(nullptr);
		if (auto emptyCursor = ::createEmptyCursor(instance))
			if (::registerWindowClass(instance, emptyCursor))
				return makeUnique<App, WindowsApp>(instance, std::move(emptyCursor));
		return nullptr;
	}
}
