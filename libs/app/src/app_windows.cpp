// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "app_windows.hpp"

#include <seir_base/unique_ptr.hpp>

namespace seir
{
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

	void WindowsApp::quit()
	{
		if (beginQuit())
			::PostQuitMessage(0);
	}

	UniquePtr<App> App::create()
	{
		return makeUnique<App, WindowsApp>();
	}
}
