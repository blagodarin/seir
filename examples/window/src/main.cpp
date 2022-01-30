// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_app/app.hpp>
#include <seir_app/events.hpp>
#include <seir_app/window.hpp>
#include <seir_u8main/u8main.hpp>

#include <thread>

namespace
{
	struct EventCallbacks : seir::EventCallbacks
	{
		void onKeyEvent(seir::Window& window, const seir::KeyEvent& event) override
		{
			if (event._key == seir::Key::Escape && event._pressed)
				window.close();
		}
	};
}

int u8main(int, char**)
{
	const auto app = seir::SharedPtr{ seir::App::create() };
	const auto window = seir::Window::create(app);
	window->show();
	for (EventCallbacks callbacks; app->processEvents(callbacks);)
		std::this_thread::sleep_for(std::chrono::milliseconds{ 1 });
	return 0;
}
