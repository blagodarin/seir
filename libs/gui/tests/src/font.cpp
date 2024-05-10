// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_gui/font.hpp>

#include <seir_app/app.hpp>
#include <seir_app/window.hpp>
#include <seir_data/blob.hpp>
#include <seir_renderer/renderer.hpp>

#include <doctest/doctest.h>

TEST_CASE("Font")
{
	const auto app = seir::SharedPtr{ seir::App::create() };
	REQUIRE(app);
	const auto window = seir::SharedPtr{ seir::Window::create(app, {}) };
	REQUIRE(window);
	const auto renderer = seir::Renderer::create(window);
	REQUIRE(renderer);
	SUBCASE("ttf file")
	{
		const auto blob = seir::Blob::from(SEIR_TEST_DIR "source_sans_pro.ttf");
		REQUIRE(blob);
		const auto font = seir::Font::load(blob, *renderer);
		REQUIRE(font);
		SUBCASE("Font::textWidth")
		{
			const auto width = font->textWidth("!", font->size());
			CHECK(width > 0);
			CHECK(width < font->size());
		}
	}
	SUBCASE("no file")
	{
		const auto font = seir::Font::load({}, *renderer);
		CHECK_FALSE(font);
	}
}
