// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_app/app.hpp>

#include <seir_base/unique_ptr.hpp>

#include <doctest/doctest.h>

TEST_CASE("App")
{
	const auto app = seir::App::create();
	REQUIRE(app);
	CHECK(app->processEvents());
	CHECK(app->processEvents());
	app->quit();
	CHECK_FALSE(app->processEvents());
}
