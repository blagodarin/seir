// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/shared_ptr.hpp>

namespace seir
{
	class EventCallbacks;

	//
	class App : public ReferenceCounter
	{
	public:
		//
		[[nodiscard]] static UniquePtr<App> create();

		// Processes application events.
		// Returns false if the application was requested to quit.
		[[nodiscard]] virtual bool processEvents(EventCallbacks&) = 0;

		//
		virtual void quit() noexcept = 0;
	};
}
