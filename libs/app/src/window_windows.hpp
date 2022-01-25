// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_app/window.hpp>

#include <seir_base/pointer.hpp>
#include <seir_base/shared_ptr.hpp>

#define WIN32_LEAN_AND_MEAN
#include <seir_base/windows_utils.hpp>

namespace seir
{
	class WindowsApp;

	struct HwndDeleter
	{
		static void free(HWND) noexcept;
	};

	using Hwnd = Pointer<std::remove_pointer_t<HWND>, HwndDeleter>;

	class WindowsWindow final : public Window
	{
	public:
		WindowsWindow(SharedPtr<WindowsApp>&&, Hwnd&&) noexcept;

		void close() noexcept override;
		void show() noexcept override;

	private:
		const SharedPtr<WindowsApp> _app;
		const Hwnd _hwnd;
	};
}
