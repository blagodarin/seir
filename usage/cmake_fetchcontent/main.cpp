// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_app/app.hpp>
#include <seir_app/events.hpp>
#include <seir_app/window.hpp>

namespace
{
	struct State : public seir::EventCallbacks
	{
		void onKeyEvent(seir::Window& window, const seir::KeyEvent& event) override
		{
			if (event._pressed)
				window.close();
		}
	};
}

int main(int, char**)
{
	seir::App app;
	seir::Window window{ app, "Example" };
	window.show();
	for (State state; app.processEvents(state);)
		;
}
