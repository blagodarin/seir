// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "helpers.hpp"

namespace seir
{
	void HcursorDeleter::free(HCURSOR handle) noexcept
	{
		if (handle && !::DestroyCursor(handle))
			if (const auto error = ::GetLastError(); error != ERROR_ALREADY_EXISTS)
				windows::reportError("DestroyCursor", error);
	}

	void HiconDeleter::free(HICON handle) noexcept
	{
		if (handle && !::DestroyIcon(handle))
			windows::reportError("DestroyIcon");
	}

	void HwndDeleter::free(HWND hwnd) noexcept
	{
		if (hwnd && !::DestroyWindow(hwnd))
			windows::reportError("DestroyWindow");
	}
}
