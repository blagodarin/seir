// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "app.hpp"

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
	App::App()
		: _impl{ std::make_unique<AppImpl>() }
	{
		// Once created, the global app object can't be uncreated, so it may already exist at this point.
		// This shouldn't happen in real apps but does happen in tests, so we'll just pretend that's the case.
		// TODO: Report some runtime warning if NSApp isn't null.
		@autoreleasepool
		{
			[NSApplication sharedApplication];
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

	bool App::processEvents(EventCallbacks&)
	{
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
