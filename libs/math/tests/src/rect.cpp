// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_math/rect.hpp>

#include <doctest/doctest.h>

using seir::Rect;
using seir::Size2D;
using seir::Vec2;

TEST_CASE("Rect::Rect()")
{
	const Rect r;
	CHECK(r.left() == 0.f);
	CHECK(r.top() == 0.f);
	CHECK(r.right() == 0.f);
	CHECK(r.bottom() == 0.f);
	CHECK(r.width() == 0.f);
	CHECK(r.height() == 0.f);
}

TEST_CASE("Rect::Rect(Vec2, Vec2)")
{
	const Rect r{ { 1, 2 }, Vec2{ 4, 7 } };
	CHECK(r.left() == 1.f);
	CHECK(r.top() == 2.f);
	CHECK(r.right() == 4.f);
	CHECK(r.bottom() == 7.f);
	CHECK(r.width() == 3.f);
	CHECK(r.height() == 5.f);
}

TEST_CASE("Rect::Rect(Vec2, Size2D)")
{
	const Rect r{ { 1, 2 }, Size2D{ 3, 5 } };
	CHECK(r.left() == 1.f);
	CHECK(r.top() == 2.f);
	CHECK(r.right() == 4.f);
	CHECK(r.bottom() == 7.f);
	CHECK(r.width() == 3.f);
	CHECK(r.height() == 5.f);
}

TEST_CASE("Rect::Rect(SizeD)")
{
	const Rect r{ Size2D{ 1, 2 } };
	CHECK(r.left() == 0.f);
	CHECK(r.top() == 0.f);
	CHECK(r.right() == 1.f);
	CHECK(r.bottom() == 2.f);
	CHECK(r.width() == 1.f);
	CHECK(r.height() == 2.f);
}

TEST_CASE("Rect::bound(Vec2)")
{
	const Rect r{ { 1, 1 }, Size2D{ 1, 1 } };

	CHECK(r.bound({ 0, 0 }) == Vec2(1, 1));
	CHECK(r.bound({ 1, 0 }) == Vec2(1, 1));
	CHECK(r.bound({ 2, 0 }) == Vec2(2, 1));
	CHECK(r.bound({ 4, 0 }) == Vec2(2, 1));

	CHECK(r.bound({ 0, 1 }) == Vec2(1, 1));
	CHECK(r.bound({ 1, 1 }) == Vec2(1, 1));
	CHECK(r.bound({ 2, 1 }) == Vec2(2, 1));
	CHECK(r.bound({ 4, 1 }) == Vec2(2, 1));

	CHECK(r.bound({ 0, 2 }) == Vec2(1, 2));
	CHECK(r.bound({ 1, 2 }) == Vec2(1, 2));
	CHECK(r.bound({ 2, 2 }) == Vec2(2, 2));
	CHECK(r.bound({ 4, 2 }) == Vec2(2, 2));

	CHECK(r.bound({ 0, 3 }) == Vec2(1, 2));
	CHECK(r.bound({ 1, 3 }) == Vec2(1, 2));
	CHECK(r.bound({ 2, 3 }) == Vec2(2, 2));
	CHECK(r.bound({ 4, 3 }) == Vec2(2, 2));
}
