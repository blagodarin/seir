// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_app/app.hpp>

#include <AppKit/AppKit.h>

@interface SeirApplicationDelegate : NSObject <NSApplicationDelegate>
@property seir::AppImpl* app;
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender;
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender;
@end

namespace seir
{
	class AppImpl
	{
	public:
		bool _quit = false;

	private:
		SeirApplicationDelegate* _delegate = nullptr;
		friend App;
	};
}
