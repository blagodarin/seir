// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_app/app.hpp>
#include <seir_base/pointer.hpp>
#include "app.hpp"

#define WIN32_LEAN_AND_MEAN
#include <seir_base/windows_utils.hpp>

namespace seir
{
	struct WindowsCursorDeleter
	{
		static void free(HCURSOR) noexcept;
	};

	using WindowsCursor = Pointer<std::remove_pointer_t<HCURSOR>, WindowsCursorDeleter>;

	class WindowsApp : public App
		, private AppHelper
	{
	public:
		WindowsApp(HINSTANCE, WindowsCursor&& emptyCursor);
		~WindowsApp() noexcept;

		[[nodiscard]] constexpr HCURSOR emptyCursor() const noexcept { return _emptyCursor; }
		[[nodiscard]] constexpr HINSTANCE instance() const noexcept { return _instance; }

		static LRESULT CALLBACK staticWindowProc(HWND, UINT, WPARAM, LPARAM) noexcept;

	protected:
		bool processEvents() override;
		void quit() noexcept override;

	private:
		LRESULT windowProc(HWND, UINT, WPARAM, LPARAM) noexcept;

	private:
		const HINSTANCE _instance;
		const WindowsCursor _emptyCursor;
	};
}
