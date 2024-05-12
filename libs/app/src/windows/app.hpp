// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_app/app.hpp>

#include "helpers.hpp"

#include <unordered_map>

namespace seir
{
	class WindowsWindow;

	class AppImpl
	{
	public:
		static constexpr const wchar_t* kWindowClass = L"Seir";

		static std::unique_ptr<AppImpl> create();
		AppImpl(HINSTANCE, Hicon&& icon, Hcursor&& emptyCursor);
		~AppImpl() noexcept;

		void addWindow(HWND, WindowsWindow*);
		[[nodiscard]] constexpr HCURSOR emptyCursor() const noexcept { return _emptyCursor; }
		[[nodiscard]] constexpr HINSTANCE instance() const noexcept { return _instance; }

		static LRESULT CALLBACK staticWindowProc(HWND, UINT, WPARAM, LPARAM) noexcept;

	private:
		LRESULT windowProc(HWND, UINT, WPARAM, LPARAM) noexcept;

	private:
		const HINSTANCE _instance;
		const Hicon _icon;
		const Hcursor _emptyCursor;
		EventCallbacks* _callbacks = nullptr;
		uint16_t _highSurrogate = 0; // High (first) code unit of UTF-16 surrogate pair.
		std::unordered_map<HWND, WindowsWindow*> _windows;
		friend App;
	};
}
