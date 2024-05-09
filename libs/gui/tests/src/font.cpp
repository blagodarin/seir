// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_gui/font.hpp>

#include <seir_data/blob.hpp>

#include <doctest/doctest.h>

TEST_CASE("Font")
{
	const auto blob = seir::Blob::from(SEIR_TEST_DIR "source_sans_pro.ttf");
	REQUIRE(blob);
	const auto font = seir::Font::load(blob);
	REQUIRE(font);
	SUBCASE("Font::textWidth")
	{
		const auto width = font->textWidth("!", font->size());
		CHECK(width > 0);
		CHECK(width < font->size());
	}
}

TEST_CASE("Font::load")
{
	const auto font = seir::Font::load({});
	CHECK_FALSE(font);
}
