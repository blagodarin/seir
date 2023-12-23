// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_graphics/rectf.hpp>

#include <doctest/doctest.h>

using seir::Point;
using seir::Rect;
using seir::RectF;
using seir::SizeF;
using seir::Vec2;

TEST_CASE("RectF::RectF()")
{
	const RectF r;
	CHECK(r.left() == 0.f);
	CHECK(r.top() == 0.f);
	CHECK(r.right() == 0.f);
	CHECK(r.bottom() == 0.f);
	CHECK(r.width() == 0.f);
	CHECK(r.height() == 0.f);
}

TEST_CASE("RectF::RectF(Vec2, Vec2)")
{
	const RectF r{ { 1, 2 }, Vec2{ 4, 7 } };
	CHECK(r.left() == 1.f);
	CHECK(r.top() == 2.f);
	CHECK(r.right() == 4.f);
	CHECK(r.bottom() == 7.f);
	CHECK(r.width() == 3.f);
	CHECK(r.height() == 5.f);
}

TEST_CASE("RectF::RectF(Vec2, SizeF)")
{
	const RectF r{ { 1, 2 }, SizeF{ 3, 5 } };
	CHECK(r.left() == 1.f);
	CHECK(r.top() == 2.f);
	CHECK(r.right() == 4.f);
	CHECK(r.bottom() == 7.f);
	CHECK(r.width() == 3.f);
	CHECK(r.height() == 5.f);
}

TEST_CASE("RectF::RectF(SizeF)")
{
	const RectF r{ SizeF{ 1, 2 } };
	CHECK(r.left() == 0.f);
	CHECK(r.top() == 0.f);
	CHECK(r.right() == 1.f);
	CHECK(r.bottom() == 2.f);
	CHECK(r.width() == 1.f);
	CHECK(r.height() == 2.f);
}

TEST_CASE("RectF::RectF(Rect)")
{
	const RectF r{ Rect{ { 1, 2 }, Point{ 4, 7 } } };
	CHECK(r.left() == 1.f);
	CHECK(r.top() == 2.f);
	CHECK(r.right() == 4.f);
	CHECK(r.bottom() == 7.f);
	CHECK(r.width() == 3.f);
	CHECK(r.height() == 5.f);
}
