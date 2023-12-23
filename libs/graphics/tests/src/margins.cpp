// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_graphics/margins.hpp>

#include <doctest/doctest.h>

using seir::Margins;

TEST_CASE("Margins::Margins()")
{
	const Margins m;
	CHECK(m._left == 0);
	CHECK(m._top == 0);
	CHECK(m._right == 0);
	CHECK(m._bottom == 0);
}

TEST_CASE("Margins::Margins(int)")
{
	const Margins m{ 1 };
	CHECK(m._left == 1);
	CHECK(m._top == 1);
	CHECK(m._right == 1);
	CHECK(m._bottom == 1);
}

TEST_CASE("Margins::Margins(int, int)")
{
	const Margins m{ 1, 2 };
	CHECK(m._left == 2);
	CHECK(m._top == 1);
	CHECK(m._right == 2);
	CHECK(m._bottom == 1);
}

TEST_CASE("Margins::Margins(int, int, int)")
{
	const Margins m{ 1, 2, 3 };
	CHECK(m._left == 2);
	CHECK(m._top == 1);
	CHECK(m._right == 2);
	CHECK(m._bottom == 3);
}

TEST_CASE("Margins::Margins(int, int, int, int)")
{
	const Margins m{ 1, 2, 3, 4 };
	CHECK(m._left == 4);
	CHECK(m._top == 1);
	CHECK(m._right == 2);
	CHECK(m._bottom == 3);
}
