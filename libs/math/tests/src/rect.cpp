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

TEST_CASE("Rect::Rect(Size2D)")
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

TEST_CASE("Rect::contains(Vec2)")
{
	const Rect r{ { 1, 2 }, Vec2{ 3, 4 } };
	SUBCASE("top left")
	{
		CHECK_FALSE(r.contains(Vec2{ 0.9, 1.9 }));
		CHECK_FALSE(r.contains(Vec2{ 0.9, 2.0 }));
		CHECK_FALSE(r.contains(Vec2{ 1.0, 1.9 }));
		CHECK(r.contains(Vec2{ 1.0, 2.0 }));
	}
	SUBCASE("top right")
	{
		CHECK_FALSE(r.contains(Vec2{ 2.9, 1.9 }));
		CHECK(r.contains(Vec2{ 2.9, 2.0 }));
		CHECK_FALSE(r.contains(Vec2{ 3.0, 1.9 }));
		CHECK_FALSE(r.contains(Vec2{ 3.0, 2.0 }));
	}
	SUBCASE("bottom left")
	{
		CHECK_FALSE(r.contains(Vec2{ 0.9, 3.9 }));
		CHECK_FALSE(r.contains(Vec2{ 0.9, 4.0 }));
		CHECK(r.contains(Vec2{ 1.0, 3.9 }));
		CHECK_FALSE(r.contains(Vec2{ 1.0, 4.0 }));
	}
	SUBCASE("bottom right")
	{
		CHECK(r.contains(Vec2{ 2.9, 3.9 }));
		CHECK_FALSE(r.contains(Vec2{ 2.9, 4.0 }));
		CHECK_FALSE(r.contains(Vec2{ 3.0, 3.9 }));
		CHECK_FALSE(r.contains(Vec2{ 3.0, 4.0 }));
	}
}

TEST_CASE("Rect::contains(Rect::bound(Vec2))")
{
	const Rect r{ { 1, 2 }, Vec2{ 3, 4 } };
	SUBCASE("top")
	{
		CHECK(r.contains(r.bound({ 0, 1 })));
		CHECK(r.contains(r.bound({ 2, 1 })));
		CHECK_FALSE(r.contains(r.bound({ 4, 1 })));
	}
	SUBCASE("middle")
	{
		CHECK(r.contains(r.bound({ 0, 3 })));
		CHECK(r.contains(r.bound({ 2, 3 })));
		CHECK_FALSE(r.contains(r.bound({ 4, 3 })));
	}
	SUBCASE("bottom")
	{
		CHECK_FALSE(r.contains(r.bound({ 0, 5 })));
		CHECK_FALSE(r.contains(r.bound({ 2, 5 })));
		CHECK_FALSE(r.contains(r.bound({ 4, 5 })));
	}
}

TEST_CASE("Rect::intersection(Rect)")
{
	const Rect r{ { 1, 2 }, Vec2{ 4, 5 } };
	SUBCASE("exact match")
	{
		const auto i = r.intersection(r);
		CHECK_FALSE(i.isEmpty());
		CHECK(i.left() == 1);
		CHECK(i.top() == 2);
		CHECK(i.right() == 4);
		CHECK(i.bottom() == 5);
	}
	SUBCASE("top left / bottom right")
	{
		SUBCASE("touch corner")
		{
			const auto i = r.intersection({ { 0, 0 }, Vec2{ 1, 2 } });
			CHECK(i.isEmpty());
			CHECK(i.left() == 1);
			CHECK(i.top() == 2);
			CHECK(i.right() == 1);
			CHECK(i.bottom() == 2);
		}
		SUBCASE("touch vertical side")
		{
			const auto i = r.intersection({ { 0, 0 }, Vec2{ 1, 3 } });
			CHECK(i.isEmpty());
			CHECK(i.left() == 1);
			CHECK(i.top() == 2);
			CHECK(i.right() == 1);
			CHECK(i.bottom() == 3);
		}
		SUBCASE("touch horizontal side")
		{
			const auto i = r.intersection({ { 0, 0 }, Vec2{ 2, 2 } });
			CHECK(i.isEmpty());
			CHECK(i.left() == 1);
			CHECK(i.top() == 2);
			CHECK(i.right() == 2);
			CHECK(i.bottom() == 2);
		}
		SUBCASE("intersect corner")
		{
			const auto i = r.intersection({ { 0, 0 }, Vec2{ 2, 3 } });
			CHECK_FALSE(i.isEmpty());
			CHECK(i.left() == 1);
			CHECK(i.top() == 2);
			CHECK(i.right() == 2);
			CHECK(i.bottom() == 3);
		}
	}
	SUBCASE("top right / bottom left")
	{
		SUBCASE("touch corner")
		{
			const auto i = r.intersection({ { 4, 0 }, Vec2{ 5, 2 } });
			CHECK(i.isEmpty());
			CHECK(i.left() == 4);
			CHECK(i.top() == 2);
			CHECK(i.right() == 4);
			CHECK(i.bottom() == 2);
		}
		SUBCASE("touch vertical side")
		{
			const auto i = r.intersection({ { 4, 0 }, Vec2{ 5, 3 } });
			CHECK(i.isEmpty());
			CHECK(i.left() == 4);
			CHECK(i.top() == 2);
			CHECK(i.right() == 4);
			CHECK(i.bottom() == 3);
		}
		SUBCASE("touch horizontal side")
		{
			const auto i = r.intersection({ { 3, 0 }, Vec2{ 5, 2 } });
			CHECK(i.isEmpty());
			CHECK(i.left() == 3);
			CHECK(i.top() == 2);
			CHECK(i.right() == 4);
			CHECK(i.bottom() == 2);
		}
		SUBCASE("intersect corner")
		{
			const auto i = r.intersection({ { 3, 0 }, Vec2{ 5, 3 } });
			CHECK_FALSE(i.isEmpty());
			CHECK(i.left() == 3);
			CHECK(i.top() == 2);
			CHECK(i.right() == 4);
			CHECK(i.bottom() == 3);
		}
	}
	SUBCASE("inside / outside")
	{
		const auto i = r.intersection({ { 0, 1 }, Vec2{ 5, 6 } });
		CHECK_FALSE(i.isEmpty());
		CHECK(i.left() == 1);
		CHECK(i.top() == 2);
		CHECK(i.right() == 4);
		CHECK(i.bottom() == 5);
	}
}

TEST_CASE("Rect::intersects(Rect)")
{
	const Rect r{ { 1, 2 }, Vec2{ 4, 5 } };
	SUBCASE("exact match")
	{
		CHECK(r.intersects(r));
	}
	SUBCASE("top left / bottom right")
	{
		SUBCASE("touch corner")
		{
			const Rect other{ { 0, 0 }, Vec2{ 1, 2 } };
			CHECK_FALSE(r.intersects(other));
			CHECK_FALSE(other.intersects(r));
		}
		SUBCASE("touch vertical side")
		{
			const Rect other{ { 0, 0 }, Vec2{ 1, 3 } };
			CHECK_FALSE(r.intersects(other));
			CHECK_FALSE(other.intersects(r));
		}
		SUBCASE("touch horizontal side")
		{
			const Rect other{ { 0, 0 }, Vec2{ 2, 2 } };
			CHECK_FALSE(r.intersects(other));
			CHECK_FALSE(other.intersects(r));
		}
		SUBCASE("intersect corner")
		{
			const Rect other{ { 0, 0 }, Vec2{ 2, 3 } };
			CHECK(r.intersects(other));
			CHECK(other.intersects(r));
		}
	}
	SUBCASE("top right / bottom left")
	{
		SUBCASE("touch corner")
		{
			const Rect other{ { 4, 0 }, Vec2{ 5, 2 } };
			CHECK_FALSE(r.intersects(other));
			CHECK_FALSE(other.intersects(r));
		}
		SUBCASE("touch vertical side")
		{
			const Rect other{ { 4, 0 }, Vec2{ 5, 3 } };
			CHECK_FALSE(r.intersects(other));
			CHECK_FALSE(other.intersects(r));
		}
		SUBCASE("touch horizontal side")
		{
			const Rect other{ { 3, 0 }, Vec2{ 5, 2 } };
			CHECK_FALSE(r.intersects(other));
			CHECK_FALSE(other.intersects(r));
		}
		SUBCASE("intersect corner")
		{
			const Rect other{ { 3, 0 }, Vec2{ 5, 3 } };
			CHECK(r.intersects(other));
			CHECK(other.intersects(r));
		}
	}
	SUBCASE("inside / outside")
	{
		const Rect other{ { 0, 1 }, Vec2{ 5, 6 } };
		CHECK(r.intersects(other));
		CHECK(other.intersects(r));
	}
}
