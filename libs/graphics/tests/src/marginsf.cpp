// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_graphics/marginsf.hpp>

#include <doctest/doctest.h>

using seir::Margins;
using seir::MarginsF;

TEST_CASE("MarginsF::MarginsF()")
{
	const MarginsF m;
	CHECK(m._left == 0.f);
	CHECK(m._top == 0.f);
	CHECK(m._right == 0.f);
	CHECK(m._bottom == 0.f);
}

TEST_CASE("MarginsF::MarginsF(float)")
{
	const MarginsF m{ 1.f };
	CHECK(m._left == 1.f);
	CHECK(m._top == 1.f);
	CHECK(m._right == 1.f);
	CHECK(m._bottom == 1.f);
}

TEST_CASE("MarginsF::MarginsF(float, float)")
{
	const MarginsF m{ 1.f, 2.f };
	CHECK(m._left == 2.f);
	CHECK(m._top == 1.f);
	CHECK(m._right == 2.f);
	CHECK(m._bottom == 1.f);
}

TEST_CASE("MarginsF::MarginsF(float, float, float)")
{
	const MarginsF m{ 1.f, 2.f, 3.f };
	CHECK(m._left == 2.f);
	CHECK(m._top == 1.f);
	CHECK(m._right == 2.f);
	CHECK(m._bottom == 3.f);
}

TEST_CASE("MarginsF::MarginsF(float, float, float, float)")
{
	const MarginsF m{ 1.f, 2.f, 3.f, 4.f };
	CHECK(m._left == 4.f);
	CHECK(m._top == 1.f);
	CHECK(m._right == 2.f);
	CHECK(m._bottom == 3.f);
}

TEST_CASE("MarginsF::MarginsF(Margins)")
{
	const MarginsF m{ Margins{ 1, 2, 3, 4 } };
	CHECK(m._left == 4.f);
	CHECK(m._top == 1.f);
	CHECK(m._right == 2.f);
	CHECK(m._bottom == 3.f);
}
