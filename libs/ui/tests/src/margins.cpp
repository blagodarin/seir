// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_ui/margins.hpp>

#include <doctest/doctest.h>

using seir::Margins;

TEST_CASE("Margins::Margins()")
{
	const Margins m;
	CHECK(m._left == 0.f);
	CHECK(m._top == 0.f);
	CHECK(m._right == 0.f);
	CHECK(m._bottom == 0.f);
}

TEST_CASE("Margins::Margins(float)")
{
	const Margins m{ 1.f };
	CHECK(m._left == 1.f);
	CHECK(m._top == 1.f);
	CHECK(m._right == 1.f);
	CHECK(m._bottom == 1.f);
}

TEST_CASE("Margins::Margins(float, float)")
{
	const Margins m{ 1.f, 2.f };
	CHECK(m._left == 2.f);
	CHECK(m._top == 1.f);
	CHECK(m._right == 2.f);
	CHECK(m._bottom == 1.f);
}

TEST_CASE("Margins::Margins(float, float, float)")
{
	const Margins m{ 1.f, 2.f, 3.f };
	CHECK(m._left == 2.f);
	CHECK(m._top == 1.f);
	CHECK(m._right == 2.f);
	CHECK(m._bottom == 3.f);
}

TEST_CASE("Margins::Margins(float, float, float, float)")
{
	const Margins m{ 1.f, 2.f, 3.f, 4.f };
	CHECK(m._left == 4.f);
	CHECK(m._top == 1.f);
	CHECK(m._right == 2.f);
	CHECK(m._bottom == 3.f);
}
