// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_app/window.hpp>

#include <AppKit/AppKit.h>
#include <QuartzCore/CAMetalLayer.h>

namespace seir
{
	class AppImpl;
}

// The delegate holds the window to prevent it from being destroyed
// after being closed, leading to dangling window pointer.
@interface SeirWindowDelegate : NSObject <NSWindowDelegate>
@property seir::AppImpl* appImpl;
@property(strong) NSWindow* window;
@property(strong) CAMetalLayer* metalLayer;
- (BOOL)windowShouldClose:(NSWindow*)window;
- (void)windowWillClose:(NSNotification*)notification;
@end

namespace seir
{
	class WindowImpl
	{
	public:
		WindowImpl(AppImpl&, Window&, SeirWindowDelegate*) noexcept;
		~WindowImpl() noexcept;

		Window& window() noexcept { return _window; }

	private:
		static std::unique_ptr<WindowImpl> create(AppImpl&, Window&, const std::string&);

	private:
		const AppImpl& _app;
		Window& _window;
		SeirWindowDelegate* _delegate;
		friend Window;
	};
}
