// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_gui/font.hpp>

#include <seir_app/app.hpp>
#include <seir_app/window.hpp>
#include <seir_io/blob.hpp>
#include <seir_renderer/renderer.hpp>

#include <doctest/doctest.h>

TEST_CASE("Font")
{
	seir::App app;
	seir::Window window{ app, {} };
	seir::Renderer renderer{ window };
	SUBCASE("ttf file")
	{
		const auto blob = seir::Blob::from(SEIR_DATA_DIR "source_sans_pro.ttf");
		REQUIRE(blob);
		const auto font = seir::Font::load(renderer, blob, 16);
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
		const auto font = seir::Font::load(renderer, {}, 16);
		CHECK_FALSE(font);
	}
	SUBCASE("bad file")
	{
		const auto blob = seir::Blob::from(SEIR_DATA_DIR "icon.ico");
		REQUIRE(blob);
		const auto font = seir::Font::load(renderer, blob, 16);
		CHECK_FALSE(font);
	}
}
