// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_app/app.hpp>

#include <seir_base/pointer.hpp>

#include <unordered_map>

#define WIN32_LEAN_AND_MEAN
#include <seir_base/windows_utils.hpp>

namespace seir
{
	class WindowsWindow;

	struct HcursorDeleter
	{
		static void free(HCURSOR) noexcept;
	};

	using Hcursor = Pointer<std::remove_pointer_t<HCURSOR>, HcursorDeleter>;

	class WindowsApp final : public App
	{
	public:
		static constexpr const wchar_t* kWindowClass = L"Seir";

		WindowsApp(HINSTANCE, Hcursor&& emptyCursor);
		~WindowsApp() noexcept;

		void addWindow(HWND, WindowsWindow*);
		[[nodiscard]] constexpr HCURSOR emptyCursor() const noexcept { return _emptyCursor; }
		[[nodiscard]] constexpr HINSTANCE instance() const noexcept { return _instance; }

		static LRESULT CALLBACK staticWindowProc(HWND, UINT, WPARAM, LPARAM) noexcept;

	private:
		bool processEvents(EventCallbacks&) override;
		void quit() noexcept override;

		LRESULT windowProc(HWND, UINT, WPARAM, LPARAM) noexcept;

	private:
		const HINSTANCE _instance;
		const Hcursor _emptyCursor;
		EventCallbacks* _callbacks = nullptr;
		uint16_t _highSurrogate = 0; // High (first) code unit of UTF-16 surrogate pair.
		std::unordered_map<HWND, WindowsWindow*> _windows;
	};
}
