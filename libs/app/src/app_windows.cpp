// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "app_windows.hpp"

#include <seir_app/events.hpp>
#include <seir_base/int_utils.hpp>
#include <seir_base/scope.hpp>
#include <seir_base/utf8.hpp>
#include "window_windows.hpp"

#include <cassert>
#include <memory>

namespace
{
	seir::Hcursor createEmptyCursor(HINSTANCE instance)
	{
		const auto width = ::GetSystemMetrics(SM_CXCURSOR);
		const auto height = ::GetSystemMetrics(SM_CYCURSOR);
		const auto maskSize = static_cast<size_t>(width * height / 8);
		const auto buffer = std::make_unique<uint8_t[]>(2 * maskSize);
		std::memset(buffer.get(), 0xff, maskSize);            // AND mask;
		std::memset(buffer.get() + maskSize, 0x00, maskSize); // XOR mask.
		if (const auto cursor = ::CreateCursor(instance, 0, 0, width, height, buffer.get(), buffer.get() + maskSize))
			return seir::Hcursor{ cursor };
		seir::windows::reportError("CreateCursor");
		return {};
	}

	seir::Key mapKey(WPARAM, LPARAM lparam) // TODO: Add key code testing utility.
	{
		using seir::Key;
		static constexpr std::array<Key, 0x60> kScanCodeTable{
			Key::None, Key::Escape, Key::_1, Key::_2, Key::_3, Key::_4, Key::_5, Key::_6,                       // 0x00 - 0x07
			Key::_7, Key::_8, Key::_9, Key::_0, Key::Minus, Key::Equals, Key::Backspace, Key::Tab,              // 0x08 - 0x0F
			Key::Q, Key::W, Key::E, Key::R, Key::T, Key::Y, Key::U, Key::I,                                     // 0x10 - 0x17
			Key::O, Key::P, Key::LBracket, Key::RBracket, Key::Enter, Key::LControl, Key::A, Key::S,            // 0x18 - 0x1F
			Key::D, Key::F, Key::G, Key::H, Key::J, Key::K, Key::L, Key::Semicolon,                             // 0x20 - 0x27
			Key::Apostrophe, Key::Grave, Key::LShift, Key::Backslash, Key::Z, Key::X, Key::C, Key::V,           // 0x28 - 0x2F
			Key::B, Key::N, Key::M, Key::Comma, Key::Period, Key::Slash, Key::RShift, Key::Multiply,            // 0x30 - 0x37
			Key::LAlt, Key::Space, Key::CapsLock, Key::F1, Key::F2, Key::F3, Key::F4, Key::F5,                  // 0x38 - 0x3F
			Key::F6, Key::F7, Key::F8, Key::F9, Key::F10, Key::Pause, Key::ScrollLock, Key::Num7,               // 0x40 - 0x47
			Key::Num8, Key::Num9, Key::Subtract, Key::Num4, Key::Num5, Key::Num6, Key::Add, Key::Num1,          // 0x48 - 0x4F
			Key::Num2, Key::Num3, Key::Num0, Key::Decimal, Key::None, Key::None, Key::NonUsBackslash, Key::F11, // 0x50 - 0x57
			Key::F12, Key::None, Key::None, Key::LGui, Key::RGui, Key::App, Key::None, Key::None,               // 0x58 - 0x5F
		};
		const auto scanCode = seir::toUnsigned((lparam >> 16) & 0xFF);
		if (scanCode >= kScanCodeTable.size())
			return Key::None;
		auto key = kScanCodeTable[scanCode];
		if (key != Key::None)
		{
			if (HIWORD(lparam) & KF_EXTENDED)
				switch (key)
				{
				case Key::Enter: key = Key::NumEnter; break;
				case Key::Slash: key = Key::Divide; break;
				case Key::Pause: key = Key::NumLock; break;
				case Key::Multiply: key = Key::PrintScreen; break; // TODO: Handle release-only behavior.
				case Key::Num1: key = Key::End; break;
				case Key::Num2: key = Key::Down; break;
				case Key::Num3: key = Key::PageDown; break;
				case Key::Num4: key = Key::Left; break;
				case Key::Num6: key = Key::Right; break;
				case Key::Num7: key = Key::Home; break;
				case Key::Num8: key = Key::Up; break;
				case Key::Num9: key = Key::PageUp; break;
				case Key::Num0: key = Key::Insert; break;
				case Key::Decimal: key = Key::Delete; break;
				case Key::App: break;
				case Key::LControl: key = Key::RControl; break;
				case Key::LShift: key = Key::RShift; break;
				case Key::LAlt: key = Key::RAlt; break;
				case Key::LGui: break;
				case Key::RGui: break;
				default: return Key::None;
#pragma warning(suppress : 4061) // enumerator '...' in switch of enum '...' is not explicitly handled by a case label
				}
			else if (key == Key::App || key == Key::LGui || key == Key::RGui)
				return Key::None;
		}
		return key;
	}

	bool registerWindowClass(HINSTANCE instance, HCURSOR cursor) noexcept
	{
		WNDCLASSEXW windowClass{ sizeof windowClass };
		windowClass.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
		windowClass.lpfnWndProc = seir::WindowsApp::staticWindowProc;
		windowClass.hInstance = instance;
		windowClass.hIcon = ::LoadIconA(nullptr, IDI_APPLICATION);
		windowClass.hCursor = cursor;
		windowClass.hbrBackground = static_cast<HBRUSH>(::GetStockObject(BLACK_BRUSH));
		windowClass.lpszClassName = seir::WindowsApp::kWindowClass;
		if (::RegisterClassExW(&windowClass))
			return true;
		seir::windows::reportError("RegisterClassExW");
		return false;
	}

	void unregisterWindowClass(HINSTANCE instance) noexcept
	{
		if (!::UnregisterClassW(seir::WindowsApp::kWindowClass, instance))
			seir::windows::reportError("UnregisterClassW");
	}
}

namespace seir
{
	void HcursorDeleter::free(HCURSOR handle) noexcept
	{
		if (handle && !::DestroyCursor(handle))
			windows::reportError("DestroyCursor");
	}

	WindowsApp::WindowsApp(HINSTANCE instance, Hcursor&& emptyCursor)
		: _instance{ instance }
		, _emptyCursor{ std::move(emptyCursor) }
	{
	}

	WindowsApp::~WindowsApp() noexcept
	{
		::unregisterWindowClass(_instance);
	}

	bool WindowsApp::processEvents(EventCallbacks& callbacks)
	{
		assert(!_callbacks);
		_callbacks = &callbacks;
		SEIR_FINALLY([this] { _callbacks = nullptr; });
		MSG msg;
		while (::PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				return false;
			::TranslateMessage(&msg);
			::DispatchMessageW(&msg);
		}
		return true;
	}

	void WindowsApp::quit() noexcept
	{
		::PostQuitMessage(0);
	}

	void WindowsApp::addWindow(HWND hwnd, WindowsWindow* window)
	{
		_windows.emplace(hwnd, window);
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
		const auto onKeyEvent = [this, hwnd](Key key, bool pressed, bool repeated) {
			if (const auto i = _windows.find(hwnd); i != _windows.end())
				_callbacks->onKeyEvent(*i->second, { key, pressed, repeated });
		};

		const auto onTextEvent = [this, hwnd](char32_t codepoint) {
			if (const auto i = _windows.find(hwnd); i != _windows.end())
			{
				std::array<char, 4> buffer;
				if (const auto bytes = writeUtf8(buffer, codepoint))
					if (const auto c = static_cast<unsigned char>(buffer[0]); c >= 0x20 && c != 0x7f)
						_callbacks->onTextEvent(*i->second, { buffer.data(), bytes });
			}
		};

		switch (msg)
		{
		case WM_DESTROY:
			if (const auto i = _windows.find(hwnd); i != _windows.end())
			{
				_windows.erase(i);
				if (_windows.empty())
					::PostQuitMessage(0);
			}
			break;

		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			if (const auto key = ::mapKey(wparam, lparam); key != Key::None)
				onKeyEvent(key, true, static_cast<bool>(HIWORD(lparam) & KF_REPEAT));
			break;

		case WM_KEYUP:
		case WM_SYSKEYUP:
			if (const auto key = ::mapKey(wparam, lparam); key != Key::None)
				onKeyEvent(key, false, false);
			break;

		case WM_CHAR:
			if (const auto utf16 = static_cast<uint16_t>(wparam); utf16 < HIGH_SURROGATE_START || utf16 > LOW_SURROGATE_END)
				onTextEvent(static_cast<char32_t>(utf16));
			else if (utf16 < LOW_SURROGATE_START)
				_highSurrogate = utf16; // NOTE: We assume that surrogate pair messages for different windows don't mix.
			else
				onTextEvent(static_cast<char32_t>(0x10000 + ((_highSurrogate - HIGH_SURROGATE_START) << 10) + (utf16 - LOW_SURROGATE_START)));
			break;

		case WM_UNICHAR:
			if (wparam == UNICODE_NOCHAR)
				return TRUE;
			onTextEvent(static_cast<char32_t>(wparam));
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
			onKeyEvent(Key::Mouse1, msg == WM_LBUTTONDOWN, false);
			break;

		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
			onKeyEvent(Key::Mouse2, msg == WM_RBUTTONDOWN, false);
			break;

		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
			onKeyEvent(Key::Mouse3, msg == WM_MBUTTONDOWN, false);
			break;

		case WM_XBUTTONDOWN:
		case WM_XBUTTONUP:
			switch (const auto pressed = msg == WM_XBUTTONDOWN; GET_XBUTTON_WPARAM(wparam))
			{
			case XBUTTON1: onKeyEvent(Key::Mouse4, pressed, false); break;
			case XBUTTON2: onKeyEvent(Key::Mouse5, pressed, false); break;
			default: return FALSE;
			}
			return TRUE;

		default:
			return ::DefWindowProcW(hwnd, msg, wparam, lparam);
		}
		return 0;
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
