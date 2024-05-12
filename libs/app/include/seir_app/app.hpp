// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <memory>

namespace seir
{
	class EventCallbacks;

	//
	class App
	{
	public:
		App();
		~App() noexcept;

		// Processes application events.
		// Returns false if the application was requested to quit.
		[[nodiscard]] bool processEvents(EventCallbacks&);

		//
		void quit() noexcept;

	private:
		const std::unique_ptr<class AppImpl> _impl;
		friend class Window;
	};
}
