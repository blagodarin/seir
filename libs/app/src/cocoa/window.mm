// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "window.hpp"

#include <seir_graphics/point.hpp>
#include <seir_graphics/size.hpp>
#include "app.hpp"

namespace
{
	bool isLastVisibleWindow(id window)
	{
		for (id otherWindow in [NSApp windows])
			if (otherWindow != window && [otherWindow isVisible])
				return false;
		return true;
	}

	NSRect mainScreenCenterRect(CGFloat width, CGFloat height)
	{
		const auto mainScreenRect = [[NSScreen mainScreen] frame];
		return NSMakeRect(
			mainScreenRect.origin.x + (mainScreenRect.size.width - width) / 2,
			mainScreenRect.origin.y + (mainScreenRect.size.height - height) / 2,
			width,
			height);
	}

	void setWindowTitle(id window, const std::string& title) noexcept
	{
		[window setTitle:[[NSString alloc] initWithBytes:title.data()
												  length:title.size()
												encoding:NSUTF8StringEncoding]];
	}
}

@implementation SeirWindowDelegate

- (BOOL)windowShouldClose:(NSWindow*)window
{
	return YES;
}

- (void)windowWillClose:(NSNotification*)notification
{
	const NSWindow* const window = [notification object];
	const SeirWindowDelegate* const delegate = [window delegate];
	delegate.appImpl->removeWindow(window);
	[window setDelegate:nil];
	if (::isLastVisibleWindow(window))
		[NSApp terminate:window];
}

@end

namespace seir
{
	WindowImpl::WindowImpl(AppImpl& app, Window& window, SeirWindowDelegate* delegate) noexcept
		: _app{ app }, _window{ window }, _delegate{ delegate }
	{
		[_delegate setAppImpl:&app];
		app.addWindow(_delegate.window, this);
	}

	WindowImpl::~WindowImpl() noexcept
	{
		@autoreleasepool
		{
			[_delegate dealloc];
		}
	}

	std::unique_ptr<WindowImpl> WindowImpl::create(AppImpl& app, Window& window, const std::string& title)
	{
		@autoreleasepool
		{
			const auto nsWindow = [[NSWindow alloc]
				initWithContentRect:(::mainScreenCenterRect(800, 600))
						  styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable)
							backing:NSBackingStoreBuffered
							  defer:NO];
			const auto delegate = [[SeirWindowDelegate alloc] init];
			nsWindow.delegate = delegate;
			::setWindowTitle(nsWindow, title);
			const auto metalLayer = [CAMetalLayer layer];
			const auto view = [nsWindow contentView];
			view.wantsLayer = YES;
			view.layer = metalLayer;
			delegate.window = nsWindow;
			delegate.metalLayer = metalLayer;
			return std::make_unique<WindowImpl>(app, window, delegate);
		}
	}

	Window::Window(App& app, const std::string& title)
		: _impl{ app._impl ? WindowImpl::create(*app._impl, *this, title) : nullptr }
	{
	}

	Window::~Window() noexcept = default;

	void Window::close() noexcept
	{
		@autoreleasepool
		{
			[_impl->_delegate.window close];
		}
	}

	std::optional<Point> Window::cursor() const noexcept
	{
		@autoreleasepool
		{
			const auto mousePoint = [_impl->_delegate.window mouseLocationOutsideOfEventStream];
			const auto windowRect = [_impl->_delegate.window contentRectForFrameRect:[_impl->_delegate.window frame]];
			return std::make_optional<Point>(mousePoint.x, windowRect.size.height - mousePoint.y);
		}
	}

	WindowDescriptor Window::descriptor() const noexcept
	{
		return { ._pointer = _impl->_delegate.metalLayer };
	}

	void Window::setIcon(const Image&) noexcept
	{
	}

	void Window::setTitle(const std::string& title) noexcept
	{
		@autoreleasepool
		{
			::setWindowTitle(_impl->_delegate.window, title);
		}
	}

	void Window::show() noexcept
	{
		@autoreleasepool
		{
			[_impl->_delegate.window makeKeyAndOrderFront:nil];
		}
	}

	Size Window::size() const noexcept
	{
		@autoreleasepool
		{
			const auto rect = [_impl->_delegate.window contentRectForFrameRect:[_impl->_delegate.window frame]];
			return { static_cast<int>(rect.size.width), static_cast<int>(rect.size.height) };
		}
	}
}
