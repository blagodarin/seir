// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_app/app.hpp>

#include <unordered_map>

#include <AppKit/AppKit.h>

@interface SeirApplicationDelegate : NSObject <NSApplicationDelegate>
@property seir::AppImpl* app;
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender;
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender;
@end

namespace seir
{
	class WindowImpl;

	class AppImpl
	{
	public:
		void addWindow(const NSWindow*, WindowImpl*);
		bool processEvent(const NSEvent*);
		void removeWindow(const NSWindow*);

	public:
		bool _quit = false;

	private:
		SeirApplicationDelegate* _delegate = nullptr;
		EventCallbacks* _callbacks = nullptr;
		std::unordered_map<const NSWindow*, WindowImpl*> _windows;
		friend App;
	};
}
