// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_app/app.hpp>

namespace seir
{
	class AppImpl
	{
	public:
		bool _quit = false;
	};

	App::App()
		: _impl{ std::make_unique<AppImpl>() }
	{
	}

	App::~App() noexcept = default;

	bool App::processEvents(EventCallbacks&)
	{
		return !_impl->_quit;
	}

	void App::quit() noexcept
	{
		_impl->_quit = true;
	}
}
