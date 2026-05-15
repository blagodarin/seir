// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "app.hpp"

#include <seir_app/events.hpp>
#include <seir_app/key.hpp>
#include <seir_base/scope.hpp>
#include "window.hpp"

#include <array>

namespace
{
	bool isModifierKeyPressed(NSUInteger flags, NSUInteger keyMask, NSUInteger otherSideMask, NSUInteger anySideMask)
	{
		// Old keyboards use a single event for both physical keys with non-side-specific mask only,
		// so we prefer non-side-specific value if it differs from the side-specific one.
		const bool anySideKeyPressed = static_cast<bool>(flags & anySideMask);
		return anySideKeyPressed != static_cast<bool>(flags & (keyMask | otherSideMask))
			? anySideKeyPressed
			: static_cast<bool>(flags & keyMask);
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
			/*  54 */ Key::RGui,
			/*  55 */ Key::LGui,
			/*  56 */ Key::LShift,
			/*  57 */ _,
			/*  58 */ Key::LAlt,
			/*  59 */ Key::LControl,
			/*  60 */ Key::RShift,
			/*  61 */ Key::RAlt,
			/*  62 */ Key::RControl,
			/*  63 */ _, // fn
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
}

@interface SeirApplication : NSApplication
- (void)sendEvent:(NSEvent*)event;
@end

@implementation SeirApplication

- (void)sendEvent:(NSEvent*)event
{
	if (!static_cast<SeirApplicationDelegate*>(self.delegate).app->processEvent(event))
		[super sendEvent:event];
}

@end

@implementation SeirApplicationDelegate

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender
{
	// Seems that this callback only gets called in standard event loops (e. g. [NSApp run]),
	// so we should implement this behavior ourselves. Leave this NO for possible other Cocoa
	// that may call it and mess with our logic. TODO: Report some runtime warning.
	return NO;
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender
{
	// Returning NSTerminateNow makes AppKit kill the app with exit() or something,
	// so we should prevent it from doing so and handle graceful termination ourselves.
	self.app->_quit = true;
	return NSTerminateCancel;
}

@end

namespace seir
{
	void AppImpl::addWindow(const NSWindow* nsWindow, WindowImpl* window)
	{
		_windows.emplace(nsWindow, window);
	}

	bool AppImpl::processEvent(const NSEvent* event)
	{
		const auto onKeyEvent = [this](NSWindow* nsWindow, Key key, bool pressed, bool repeated) {
			if (const auto i = _windows.find(nsWindow); i != _windows.end())
				_callbacks->onKeyEvent(i->second->window(),
					{ key, pressed, repeated });
		};

		const auto onTextEvent = [this](NSWindow* nsWindow, std::string_view text) {
			if (const auto i = _windows.find(nsWindow); i != _windows.end())
				_callbacks->onTextEvent(i->second->window(), text);
		};

		assert(_callbacks);
		switch (const auto eventType = [event type])
		{
		case NSEventTypeLeftMouseDown:
		case NSEventTypeLeftMouseUp:
			if (const auto window = [event window])
				if (NSMouseInRect([event locationInWindow], [[window contentView] frame], NO))
					onKeyEvent(window, Key::Mouse1, eventType == NSEventTypeLeftMouseDown, false);
			break;

		case NSEventTypeRightMouseDown:
		case NSEventTypeRightMouseUp:
			if (const auto window = [event window])
				if (NSMouseInRect([event locationInWindow], [[window contentView] frame], NO))
					onKeyEvent(window, Key::Mouse2, eventType == NSEventTypeRightMouseDown, false);
			break;

		case NSEventTypeKeyDown:
			if (const auto key = ::mapKey([event keyCode]); key != Key::None)
				onKeyEvent([event window], key, true, [event isARepeat] == YES);
			if (const auto characters = [event characters];[characters length] > 0)
				onTextEvent([event window], [characters UTF8String]);
			break;

		case NSEventTypeKeyUp:
			if (const auto key = ::mapKey([event keyCode]); key != Key::None)
				onKeyEvent([event window], key, false, false);
			break;

		case NSEventTypeFlagsChanged:
			if (const auto i = _windows.find([event window]); i != _windows.end())
			{
				auto& window = i->second->window();
				const auto flags = [event modifierFlags];
				_callbacks->onKeyEvent(window, { Key::LControl, ::isModifierKeyPressed(flags, NX_DEVICELCTLKEYMASK, NX_DEVICERCTLKEYMASK, NX_CONTROLMASK) });
				_callbacks->onKeyEvent(window, { Key::LShift, ::isModifierKeyPressed(flags, NX_DEVICELSHIFTKEYMASK, NX_DEVICERSHIFTKEYMASK, NX_SHIFTMASK) });
				_callbacks->onKeyEvent(window, { Key::LAlt, ::isModifierKeyPressed(flags, NX_DEVICELALTKEYMASK, NX_DEVICERALTKEYMASK, NX_ALTERNATEMASK) });
				_callbacks->onKeyEvent(window, { Key::LGui, ::isModifierKeyPressed(flags, NX_DEVICELCMDKEYMASK, NX_DEVICERCMDKEYMASK, NX_COMMANDMASK) });
				_callbacks->onKeyEvent(window, { Key::RControl, ::isModifierKeyPressed(flags, NX_DEVICERCTLKEYMASK, NX_DEVICELCTLKEYMASK, NX_CONTROLMASK) });
				_callbacks->onKeyEvent(window, { Key::RShift, ::isModifierKeyPressed(flags, NX_DEVICERSHIFTKEYMASK, NX_DEVICELSHIFTKEYMASK, NX_SHIFTMASK) });
				_callbacks->onKeyEvent(window, { Key::RAlt, ::isModifierKeyPressed(flags, NX_DEVICERALTKEYMASK, NX_DEVICELALTKEYMASK, NX_ALTERNATEMASK) });
				_callbacks->onKeyEvent(window, { Key::RGui, ::isModifierKeyPressed(flags, NX_DEVICERCMDKEYMASK, NX_DEVICELCMDKEYMASK, NX_COMMANDMASK) });
			}
			break;

		default:
			return false;
		}
		return true;
	}

	void AppImpl::removeWindow(const NSWindow* nsWindow)
	{
		_windows.erase(nsWindow);
	}

	App::App()
		: _impl{ std::make_unique<AppImpl>() }
	{
		// Once created, the global app object can't be uncreated, so it may already exist at this point.
		// This shouldn't happen in real apps but does happen in tests, so we'll just pretend that's the case.
		// TODO: Report some runtime warning if NSApp isn't null.
		@autoreleasepool
		{
			[SeirApplication sharedApplication];
			assert(NSApp);
			_impl->_delegate = [[SeirApplicationDelegate alloc] init];
			[_impl->_delegate setApp:_impl.get()];
			[NSApp setDelegate:_impl->_delegate];
			[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
			[NSApp finishLaunching]; // Created windows stay in background without this.
		}
	}

	App::~App() noexcept
	{
		@autoreleasepool
		{
			[NSApp setDelegate:nil];
		}
		[_impl->_delegate dealloc]; // Trying to do this in @autoreleasepool results in SIGSEGV.
	}

	bool App::processEvents(EventCallbacks& callbacks)
	{
		assert(!_impl->_callbacks);
		_impl->_callbacks = &callbacks;
		SEIR_FINALLY{ [this]() noexcept { _impl->_callbacks = nullptr; } };
		while (!_impl->_quit)
		{
			@autoreleasepool
			{
				const auto event = [NSApp
					nextEventMatchingMask:NSEventMaskAny
								untilDate:nil
								   inMode:NSDefaultRunLoopMode
								  dequeue:YES];
				if (!event)
					break;
				[NSApp sendEvent:event];
				[NSApp updateWindows];
			}
		}
		return !_impl->_quit;
	}

	void App::quit() noexcept
	{
		@autoreleasepool
		{
			[NSApp terminate:nil];
		}
	}
}
