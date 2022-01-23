// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

namespace seir
{
	template <class>
	class UniquePtr;

	//
	class App
	{
	public:
		//
		[[nodiscard]] static UniquePtr<App> create();

		virtual ~App() noexcept = default;

		// Processes application events.
		// Returns false if the application was requested to quit.
		[[nodiscard]] virtual bool processEvents() = 0;

		//
		virtual void quit() = 0;
	};
}
