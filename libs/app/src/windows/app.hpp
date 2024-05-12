// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_app/app.hpp>

#include "helpers.hpp"

#include <unordered_map>

namespace seir
{
	class WindowImpl;

	class AppImpl
	{
	public:
		static constexpr const wchar_t* kWindowClass = L"Seir";

		AppImpl(HINSTANCE, Hicon&& icon, Hcursor&& emptyCursor);
		~AppImpl() noexcept;

		void addWindow(HWND, WindowImpl*);
		[[nodiscard]] constexpr HINSTANCE instance() const noexcept { return _instance; }

		static LRESULT CALLBACK staticWindowProc(HWND, UINT, WPARAM, LPARAM) noexcept;

	private:
		LRESULT windowProc(HWND, UINT, WPARAM, LPARAM) noexcept;

		static std::unique_ptr<AppImpl> create();

	private:
		const HINSTANCE _instance;
		const Hicon _icon;
		const Hcursor _emptyCursor;
		EventCallbacks* _callbacks = nullptr;
		uint16_t _highSurrogate = 0; // High (first) code unit of UTF-16 surrogate pair.
		std::unordered_map<HWND, WindowImpl*> _windows;
		friend App;
	};
}
