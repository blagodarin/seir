// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_app/app.hpp>

#include <seir_app/events.hpp>
#include <seir_base/unique_ptr.hpp>

#include <doctest/doctest.h>

namespace
{
	struct EventCallbacks : seir::EventCallbacks
	{
	};
}

TEST_CASE("App")
{
	const auto app = seir::App::create();
	REQUIRE(app);
	EventCallbacks callbacks;
	CHECK(app->processEvents(callbacks));
	CHECK(app->processEvents(callbacks));
	app->quit();
	CHECK_FALSE(app->processEvents(callbacks));
}
