// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_app/app.hpp>
#include <seir_app/events.hpp>
#include <seir_app/window.hpp>
#include <seir_renderer/renderer.hpp>
#include <seir_u8main/u8main.hpp>

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
	// TODO: Make less verbose.
	// Posible options include:
	// - making create() return SharedPtr;
	// - removing SharedPtr and passing App and Window by reference.
	const auto app = seir::SharedPtr{ seir::App::create() };
	const auto window = seir::SharedPtr{ seir::Window::create(app, "Window") };
	const auto renderer = seir::Renderer::create(window);
	window->show();
	for (EventCallbacks callbacks; app->processEvents(callbacks);)
		renderer->draw();
	return 0;
}
