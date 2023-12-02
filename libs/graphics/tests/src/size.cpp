// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_graphics/size.hpp>

#include <doctest/doctest.h>

using seir::Size;

TEST_CASE("Size::Size()")
{
	Size s;
	CHECK(s._width == 0);
	CHECK(s._height == 0);
}

TEST_CASE("Size::Size(int, int)")
{
	Size s{ 1, 2 };
	CHECK(s._width == 1);
	CHECK(s._height == 2);
}
