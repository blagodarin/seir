// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_graphics/rect.hpp>

#include <doctest/doctest.h>

using seir::Point;
using seir::Rect;
using seir::Size;

TEST_CASE("Rect::Rect()")
{
	Rect r;
	CHECK(r.left() == 0);
	CHECK(r.top() == 0);
	CHECK(r.right() == 0);
	CHECK(r.bottom() == 0);
	CHECK(r.width() == 0);
	CHECK(r.height() == 0);
}

TEST_CASE("Rect::Rect(Point, Point)")
{
	Rect r{ { 1, 2 }, Point{ 4, 7 } };
	CHECK(r.left() == 1);
	CHECK(r.top() == 2);
	CHECK(r.right() == 4);
	CHECK(r.bottom() == 7);
	CHECK(r.width() == 3);
	CHECK(r.height() == 5);
}

TEST_CASE("Rect::Rect(Point, Size)")
{
	Rect r{ { 1, 2 }, Size{ 3, 5 } };
	CHECK(r.left() == 1);
	CHECK(r.top() == 2);
	CHECK(r.right() == 4);
	CHECK(r.bottom() == 7);
	CHECK(r.width() == 3);
	CHECK(r.height() == 5);
}

TEST_CASE("Rect::Rect(Size)")
{
	Rect r{ Size{ 1, 2 } };
	CHECK(r.left() == 0);
	CHECK(r.top() == 0);
	CHECK(r.right() == 1);
	CHECK(r.bottom() == 2);
	CHECK(r.width() == 1);
	CHECK(r.height() == 2);
}

TEST_CASE("Rect::bound(Point)")
{
	const Rect rect{ { 1, 1 }, Size{ 2, 2 } };

	CHECK(rect.bound({ 0, 0 }) == Point(1, 1));
	CHECK(rect.bound({ 1, 0 }) == Point(1, 1));
	CHECK(rect.bound({ 2, 0 }) == Point(2, 1));
	CHECK(rect.bound({ 4, 0 }) == Point(2, 1));

	CHECK(rect.bound({ 0, 1 }) == Point(1, 1));
	CHECK(rect.bound({ 1, 1 }) == Point(1, 1));
	CHECK(rect.bound({ 2, 1 }) == Point(2, 1));
	CHECK(rect.bound({ 4, 1 }) == Point(2, 1));

	CHECK(rect.bound({ 0, 2 }) == Point(1, 2));
	CHECK(rect.bound({ 1, 2 }) == Point(1, 2));
	CHECK(rect.bound({ 2, 2 }) == Point(2, 2));
	CHECK(rect.bound({ 4, 2 }) == Point(2, 2));

	CHECK(rect.bound({ 0, 3 }) == Point(1, 2));
	CHECK(rect.bound({ 1, 3 }) == Point(1, 2));
	CHECK(rect.bound({ 2, 3 }) == Point(2, 2));
	CHECK(rect.bound({ 4, 3 }) == Point(2, 2));
}

TEST_CASE("Rect::center()")
{
	CHECK(Rect({ 0, 0 }, Size{ 0, 0 }).center() == Point(0, 0));
	CHECK(Rect({ 0, 0 }, Size{ 1, 1 }).center() == Point(0, 0));
	CHECK(Rect({ 0, 0 }, Size{ 2, 2 }).center() == Point(1, 1));
	CHECK(Rect({ 0, 0 }, Size{ 3, 3 }).center() == Point(1, 1));

	CHECK(Rect({ 1, 2 }, Size{ 3, 4 }).center() == Point(2, 4));
}

TEST_CASE("Rect::centeredAt(Rect)")
{
	const Rect odd{ { 0, 0 }, Size{ 3, 5 } };
	const Rect even{ { 0, 0 }, Size{ 6, 8 } };
	CHECK(odd.centeredAt(even) == Rect({ 1, 1 }, Size{ 3, 5 }));
	CHECK(even.centeredAt(odd) == Rect({ -1, -1 }, Size{ 6, 8 }));
}

TEST_CASE("Rect::contains(Point)")
{
	SUBCASE("width > 0 && height > 0")
	{
		const Rect rect{ { 1, 2 }, Size{ 2, 2 } };

		CHECK_FALSE(rect.contains(Point{ 0, 1 }));
		CHECK_FALSE(rect.contains(Point{ 1, 1 }));
		CHECK_FALSE(rect.contains(Point{ 2, 1 }));
		CHECK_FALSE(rect.contains(Point{ 3, 1 }));

		CHECK_FALSE(rect.contains(Point{ 0, 2 }));
		CHECK(rect.contains(Point{ 1, 2 }));
		CHECK(rect.contains(Point{ 2, 2 }));
		CHECK_FALSE(rect.contains(Point{ 3, 2 }));

		CHECK_FALSE(rect.contains(Point{ 0, 3 }));
		CHECK(rect.contains(Point{ 1, 3 }));
		CHECK(rect.contains(Point{ 2, 3 }));
		CHECK_FALSE(rect.contains(Point{ 3, 3 }));

		CHECK_FALSE(rect.contains(Point{ 0, 4 }));
		CHECK_FALSE(rect.contains(Point{ 1, 4 }));
		CHECK_FALSE(rect.contains(Point{ 2, 4 }));
		CHECK_FALSE(rect.contains(Point{ 3, 4 }));
	}
	SUBCASE("width == 0 && height == 0")
	{
		const Rect rect{ { 1, 2 }, Size{} };

		CHECK_FALSE(rect.contains(Point{ 0, 1 }));
		CHECK_FALSE(rect.contains(Point{ 1, 1 }));
		CHECK_FALSE(rect.contains(Point{ 2, 1 }));

		CHECK_FALSE(rect.contains(Point{ 0, 2 }));
		CHECK_FALSE(rect.contains(Point{ 1, 2 }));
		CHECK_FALSE(rect.contains(Point{ 2, 2 }));

		CHECK_FALSE(rect.contains(Point{ 0, 3 }));
		CHECK_FALSE(rect.contains(Point{ 1, 3 }));
		CHECK_FALSE(rect.contains(Point{ 2, 3 }));
	}
}

TEST_CASE("Rect::contains(Rect)")
{
	SUBCASE("width > 0 && height > 0")
	{
		const Rect rect{ { 1, 2 }, Size{ 4, 4 } };

		CHECK_FALSE(rect.contains({ { 0, 1 }, Size{ 2, 2 } }));
		CHECK_FALSE(rect.contains({ { 1, 1 }, Size{ 2, 2 } }));
		CHECK_FALSE(rect.contains({ { 2, 1 }, Size{ 2, 2 } }));
		CHECK_FALSE(rect.contains({ { 3, 1 }, Size{ 2, 2 } }));
		CHECK_FALSE(rect.contains({ { 4, 1 }, Size{ 2, 2 } }));

		CHECK_FALSE(rect.contains({ { 0, 2 }, Size{ 2, 2 } }));
		CHECK(rect.contains({ { 1, 2 }, Size{ 2, 2 } }));
		CHECK(rect.contains({ { 2, 2 }, Size{ 2, 2 } }));
		CHECK(rect.contains({ { 3, 2 }, Size{ 2, 2 } }));
		CHECK_FALSE(rect.contains({ { 4, 2 }, Size{ 2, 2 } }));

		CHECK_FALSE(rect.contains({ { 0, 3 }, Size{ 2, 2 } }));
		CHECK(rect.contains({ { 1, 3 }, Size{ 2, 2 } }));
		CHECK(rect.contains({ { 2, 3 }, Size{ 2, 2 } }));
		CHECK(rect.contains({ { 3, 3 }, Size{ 2, 2 } }));
		CHECK_FALSE(rect.contains({ { 4, 3 }, Size{ 2, 2 } }));

		CHECK_FALSE(rect.contains({ { 0, 4 }, Size{ 2, 2 } }));
		CHECK(rect.contains({ { 1, 4 }, Size{ 2, 2 } }));
		CHECK(rect.contains({ { 2, 4 }, Size{ 2, 2 } }));
		CHECK(rect.contains({ { 3, 4 }, Size{ 2, 2 } }));
		CHECK_FALSE(rect.contains({ { 4, 4 }, Size{ 2, 2 } }));

		CHECK_FALSE(rect.contains({ { 0, 5 }, Size{ 2, 2 } }));
		CHECK_FALSE(rect.contains({ { 1, 5 }, Size{ 2, 2 } }));
		CHECK_FALSE(rect.contains({ { 2, 5 }, Size{ 2, 2 } }));
		CHECK_FALSE(rect.contains({ { 3, 5 }, Size{ 2, 2 } }));
		CHECK_FALSE(rect.contains({ { 4, 5 }, Size{ 2, 2 } }));

		CHECK(rect.contains(rect));
		CHECK_FALSE(rect.contains({ { 0, 1 }, Size{ 6, 6 } }));
		CHECK(Rect({ 0, 1 }, Size{ 6, 6 }).contains(rect));
	}
	SUBCASE("width == 0 && height == 0")
	{
		const Rect rect{ { 1, 2 }, Size{} };
		CHECK(rect.contains(rect)); // NOTE!
		CHECK_FALSE(rect.contains({ { 0, 1 }, Size{ 2, 2 } }));
		CHECK(Rect({ 0, 1 }, Size{ 2, 2 }).contains(rect)); // NOTE!
	}
}

TEST_CASE("Rect::intersected(Rect)")
{
	const Size size{ 2, 2 };
	const Rect rect{ { 3, 3 }, size };

	CHECK(rect.intersected(Rect{ { 0, 0 }, size }) == Rect({ 3, 3 }, Size{ -1, -1 }));
	CHECK(rect.intersected(Rect{ { 1, 0 }, size }) == Rect({ 3, 3 }, Size{ 0, -1 }));
	CHECK(rect.intersected(Rect{ { 2, 0 }, size }) == Rect({ 3, 3 }, Size{ 1, -1 }));
	CHECK(rect.intersected(Rect{ { 3, 0 }, size }) == Rect({ 3, 3 }, Size{ 2, -1 }));
	CHECK(rect.intersected(Rect{ { 4, 0 }, size }) == Rect({ 4, 3 }, Size{ 1, -1 }));
	CHECK(rect.intersected(Rect{ { 5, 0 }, size }) == Rect({ 5, 3 }, Size{ 0, -1 }));
	CHECK(rect.intersected(Rect{ { 6, 0 }, size }) == Rect({ 6, 3 }, Size{ -1, -1 }));

	CHECK(rect.intersected(Rect{ { 0, 1 }, size }) == Rect({ 3, 3 }, Size{ -1, 0 }));
	CHECK(rect.intersected(Rect{ { 1, 1 }, size }) == Rect({ 3, 3 }, Size{ 0, 0 }));
	CHECK(rect.intersected(Rect{ { 2, 1 }, size }) == Rect({ 3, 3 }, Size{ 1, 0 }));
	CHECK(rect.intersected(Rect{ { 3, 1 }, size }) == Rect({ 3, 3 }, Size{ 2, 0 }));
	CHECK(rect.intersected(Rect{ { 4, 1 }, size }) == Rect({ 4, 3 }, Size{ 1, 0 }));
	CHECK(rect.intersected(Rect{ { 5, 1 }, size }) == Rect({ 5, 3 }, Size{ 0, 0 }));
	CHECK(rect.intersected(Rect{ { 6, 1 }, size }) == Rect({ 6, 3 }, Size{ -1, 0 }));
}

TEST_CASE("Rect::intersects(Rect)")
{
	const auto pass = [](const Rect& rect, const Point& topLeft, const Size& size) {
		const Rect other{ topLeft, size };
		CHECK(rect.intersects(other));
		CHECK(other.intersects(rect));
	};

	const auto fail = [](const Rect& rect, const Point& topLeft, const Size& size) {
		const Rect other{ topLeft, size };
		CHECK_FALSE(rect.intersects(other));
		CHECK_FALSE(other.intersects(rect));
	};

	SUBCASE("width > 0 && height > 0")
	{
		const Rect rect{ { 2, 3 }, Size{ 4, 6 } };

		for (int x = 0; x <= 6; ++x)
			fail(rect, { x, 1 }, { 2, 2 });

		for (int y = 2; y <= 8; ++y)
		{
			fail(rect, { 0, y }, { 2, 2 });
			for (int x = 1; x <= 5; ++x)
				pass(rect, { x, y }, { 2, 2 });
			fail(rect, { 6, y }, { 2, 2 });
		}

		for (int x = 0; x <= 6; ++x)
			fail(rect, { x, 9 }, { 2, 2 });

		CHECK(rect.intersects(rect));

		pass(rect, { 0, 1 }, { 6, 6 });
	}
	SUBCASE("width == 0 && height == 0")
	{
		const Rect rect{ { 1, 2 }, Size{} };

		fail(rect, { 0, 1 }, {});
		fail(rect, { 1, 1 }, {});
		fail(rect, { 2, 1 }, {});
		fail(rect, { 0, 2 }, {});
		pass(rect, { 1, 2 }, {}); // NOTE!
		fail(rect, { 2, 2 }, {});
		fail(rect, { 0, 3 }, {});
		fail(rect, { 1, 3 }, {});
		fail(rect, { 2, 3 }, {});

		fail(rect, { 0, 1 }, { 1, 1 });
		fail(rect, { 1, 1 }, { 1, 1 });
		fail(rect, { 0, 2 }, { 1, 1 });
		fail(rect, { 1, 2 }, { 1, 1 });

		pass(rect, { 0, 1 }, { 2, 2 }); // NOTE!
	}
}
