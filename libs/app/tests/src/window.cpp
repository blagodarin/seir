// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_app/window.hpp>

#include <seir_app/app.hpp>

#include <doctest/doctest.h>

TEST_CASE("Window")
{
	const auto app = seir::SharedPtr{ seir::App::create() };
	REQUIRE(app);
	const auto window = seir::Window::create(app);
	CHECK(window);
}
