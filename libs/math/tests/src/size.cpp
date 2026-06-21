// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_math/size.hpp>

#include <doctest/doctest.h>

using seir::Size2D;

TEST_CASE("Size2D::Size2D()")
{
	const Size2D s;
	CHECK(s.width == 0.f);
	CHECK(s.height == 0.f);
}

TEST_CASE("Size2D::Size2D(float, float)")
{
	const Size2D s{ 1.f, 2.f };
	CHECK(s.width == 1.f);
	CHECK(s.height == 2.f);
}
