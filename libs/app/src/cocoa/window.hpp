// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_app/window.hpp>

#include <AppKit/AppKit.h>

@interface SeirWindow : NSWindow
@end

// The delegate holds the window to prevent it from being destroyed
// after being closed, leading to dangling window pointer.
@interface SeirWindowDelegate : NSObject <NSWindowDelegate>
@property(strong) SeirWindow* window;
- (BOOL)windowShouldClose:(NSWindow*)window;
- (void)windowWillClose:(NSNotification*)notification;
@end

namespace seir
{
	class AppImpl;

	class WindowImpl
	{
	public:
		WindowImpl(AppImpl& app, Window& window, SeirWindowDelegate* delegate) noexcept
			: _app{ app }, _window{ window }, _delegate{ delegate } {}
		~WindowImpl() noexcept;

	private:
		static std::unique_ptr<WindowImpl> create(AppImpl&, Window&, const std::string&);

	private:
		const AppImpl& _app;
		Window& _window;
		SeirWindowDelegate* _delegate;
		friend Window;
	};
}
