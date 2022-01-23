// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_app/app.hpp>
#include "app.hpp"

#include <seir_base/unique_ptr.hpp>

namespace
{
	class StubApp : public seir::App
		, private seir::AppHelper
	{
	public:
		bool processEvents() override
		{
			if (!hasQuit())
			{
				if (!_quit)
					return true;
				endQuit();
			}
			return false;
		}

		void quit() noexcept override
		{
			if (beginQuit())
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
