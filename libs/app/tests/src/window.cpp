// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_app/window.hpp>

#include <seir_app/app.hpp>
#include <seir_app/events.hpp>

#include <doctest/doctest.h>

namespace
{
	struct EventCallbacks : seir::EventCallbacks
	{
	};
}

TEST_CASE("Window")
{
	seir::App app;
	seir::Window window{ app, {} };
	EventCallbacks callbacks;
	CHECK(app.processEvents(callbacks));
	CHECK(app.processEvents(callbacks));
	window.close();
	CHECK_FALSE(app.processEvents(callbacks));
}
