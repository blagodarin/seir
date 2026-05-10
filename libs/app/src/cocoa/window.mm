// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "window.hpp"

#include <seir_app/app.hpp>
#include <seir_app/key.hpp>
#include <seir_graphics/point.hpp>
#include <seir_graphics/size.hpp>
#include "app.hpp"

#include <array>

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

	seir::Key mapKey(uint16_t keyCode)
	{
		using seir::Key;
		constexpr auto _ = Key::None;
		static constexpr std::array kScanCodeTable{
			/*   0 */ Key::A,
			/*   1 */ Key::S,
			/*   2 */ Key::D,
			/*   3 */ Key::F,
			/*   4 */ Key::H,
			/*   5 */ Key::G,
			/*   6 */ Key::Z,
			/*   7 */ Key::X,
			/*   8 */ Key::C,
			/*   9 */ Key::V,
			/*  10 */ Key::Grave,
			/*  11 */ Key::B,
			/*  12 */ Key::Q,
			/*  13 */ Key::W,
			/*  14 */ Key::E,
			/*  15 */ Key::R,
			/*  16 */ Key::Y,
			/*  17 */ Key::T,
			/*  18 */ Key::_1,
			/*  19 */ Key::_2,
			/*  20 */ Key::_3,
			/*  21 */ Key::_4,
			/*  22 */ Key::_6,
			/*  23 */ Key::_5,
			/*  24 */ Key::Equals,
			/*  25 */ Key::_9,
			/*  26 */ Key::_7,
			/*  27 */ Key::Minus,
			/*  28 */ Key::_8,
			/*  29 */ Key::_0,
			/*  30 */ Key::RBracket,
			/*  31 */ Key::O,
			/*  32 */ Key::U,
			/*  33 */ Key::LBracket,
			/*  34 */ Key::I,
			/*  35 */ Key::P,
			/*  36 */ Key::Enter,
			/*  37 */ Key::L,
			/*  38 */ Key::J,
			/*  39 */ Key::Apostrophe,
			/*  40 */ Key::K,
			/*  41 */ Key::Semicolon,
			/*  42 */ Key::Backslash,
			/*  43 */ Key::Comma,
			/*  44 */ Key::Slash,
			/*  45 */ Key::N,
			/*  46 */ Key::M,
			/*  47 */ Key::Period,
			/*  48 */ Key::Tab,
			/*  49 */ Key::Space,
			/*  50 */ Key::NonUsBackslash,
			/*  51 */ Key::Backspace,
			/*  52 */ _,
			/*  53 */ Key::Escape,
			/*  54 */ _,
			/*  55 */ _,
			/*  56 */ _,
			/*  57 */ _,
			/*  58 */ _,
			/*  59 */ _,
			/*  60 */ _,
			/*  61 */ _,
			/*  62 */ _,
			/*  63 */ _,
			/*  64 */ _,
			/*  65 */ _,
			/*  66 */ _,
			/*  67 */ _,
			/*  68 */ _,
			/*  69 */ _,
			/*  70 */ _,
			/*  71 */ _,
			/*  72 */ _,
			/*  73 */ _,
			/*  74 */ _,
			/*  75 */ _,
			/*  76 */ _,
			/*  77 */ _,
			/*  78 */ _,
			/*  79 */ _,
			/*  80 */ _,
			/*  81 */ _,
			/*  82 */ _,
			/*  83 */ _,
			/*  84 */ _,
			/*  85 */ _,
			/*  86 */ _,
			/*  87 */ _,
			/*  88 */ _,
			/*  89 */ _,
			/*  90 */ _,
			/*  91 */ _,
			/*  92 */ _,
			/*  93 */ _,
			/*  94 */ _,
			/*  95 */ _,
			/*  96 */ Key::F5,
			/*  97 */ Key::F6,
			/*  98 */ Key::F7,
			/*  99 */ Key::F3,
			/* 100 */ Key::F8,
			/* 101 */ Key::F9,
			/* 102 */ _,
			/* 103 */ _,
			/* 104 */ _,
			/* 105 */ _,
			/* 106 */ _,
			/* 107 */ _,
			/* 108 */ _,
			/* 109 */ Key::F10,
			/* 110 */ _,
			/* 111 */ Key::F12,
			/* 112 */ _,
			/* 113 */ _,
			/* 114 */ _,
			/* 115 */ _,
			/* 116 */ _,
			/* 117 */ _,
			/* 118 */ Key::F4,
			/* 119 */ _,
			/* 120 */ Key::F2,
			/* 121 */ _,
			/* 122 */ Key::F1,
			/* 123 */ Key::Left,
			/* 124 */ Key::Right,
			/* 125 */ Key::Down,
			/* 126 */ Key::Up,
			/* 127 */ _,
		};
		return keyCode < kScanCodeTable.size() ? kScanCodeTable[keyCode] : _;
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
	if (const auto keyCode = [event keyCode]; ::mapKey(keyCode) == seir::Key::Escape)
		[self close];
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
