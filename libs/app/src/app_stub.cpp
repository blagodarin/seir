// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_app/app.hpp>

namespace
{
	class StubApp final : public seir::App
	{
	public:
		bool processEvents() override
		{
			return !_quit;
		}

		void quit() noexcept override
		{
			_quit = true;
		}

	private:
		bool _quit = false;
	};
}

namespace seir
{
	UniquePtr<App> App::create()
	{
		return makeUnique<App, StubApp>();
	}
}
