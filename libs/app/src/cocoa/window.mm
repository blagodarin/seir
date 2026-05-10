// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "window.hpp"

#include <seir_app/app.hpp>
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

@implementation SeirWindow

- (void)keyDown:(NSEvent*)event
{
	switch ([event keyCode])
	{
	case 53: // Escape.
		[self close];
		break;
	default:
		[super keyDown:event];
	}
}

@end

@implementation SeirWindowDelegate

- (BOOL)windowShouldClose:(NSWindow*)window
{
	return YES;
}

- (void)windowWillClose:(NSNotification*)notification
{
	id window = [notification object];
	[window setDelegate:nil];
	if (::isLastVisibleWindow(window))
		[NSApp terminate:window];
}

@end

namespace seir
{
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
			const auto delegate = [[SeirWindowDelegate alloc] init];
			const auto cocoaWindow = [[SeirWindow alloc]
				initWithContentRect:(::mainScreenCenterRect(800, 600))
						  styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable)
							backing:NSBackingStoreBuffered
							  defer:NO];
			cocoaWindow.delegate = delegate;
			::setWindowTitle(cocoaWindow, title);
			const auto metalLayer = [CAMetalLayer layer];
			const auto view = [cocoaWindow contentView];
			view.wantsLayer = YES;
			view.layer = metalLayer;
			delegate.window = cocoaWindow;
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
		return { _impl->_delegate.metalLayer, 0 };
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
