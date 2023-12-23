// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_graphics/sizef.hpp>

#include <doctest/doctest.h>

using seir::Size;
using seir::SizeF;

TEST_CASE("SizeF::SizeF()")
{
	const SizeF s;
	CHECK(s._width == 0.f);
	CHECK(s._height == 0.f);
}

TEST_CASE("SizeF::SizeF(int, int)")
{
	const SizeF s{ 1.f, 2.f };
	CHECK(s._width == 1.f);
	CHECK(s._height == 2.f);
}

TEST_CASE("SizeF::SizeF(Size)")
{
	const SizeF s{ Size{ 1, 2 } };
	CHECK(s._width == 1.f);
	CHECK(s._height == 2.f);
}
