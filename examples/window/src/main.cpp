// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_app/app.hpp>
#include <seir_app/window.hpp>
#include <seir_u8main/u8main.hpp>

#include <thread>

int u8main(int, char**)
{
	const auto app = seir::SharedPtr{ seir::App::create() };
	const auto window = seir::Window::create(app);
	while (app->processEvents())
		std::this_thread::sleep_for(std::chrono::milliseconds{ 1 });
	return 0;
}
