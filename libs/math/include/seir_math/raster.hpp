// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

namespace seir
{
	class Point
	{
	public:
		int _x = 0;
		int _y = 0;

		constexpr Point() noexcept = default;
		constexpr Point(int x, int y) noexcept
			: _x{ x }, _y{ y } {}
	};

	class Size
	{
	public:
		int _width = 0;
		int _height = 0;

		constexpr Size() noexcept = default;
		constexpr Size(int width, int height) noexcept
			: _width{ width }, _height{ height } {}
	};

	class Rect
	{
	public:
		constexpr Rect() noexcept = default;
		constexpr Rect(const Point& topLeft, const Point& bottomRight) noexcept
			: _left{ topLeft._x }, _top{ topLeft._y }, _right{ bottomRight._x }, _bottom{ bottomRight._y } {}
		constexpr Rect(const Point& topLeft, const Size& size) noexcept
			: _left{ topLeft._x }, _top{ topLeft._y }, _right{ _left + size._width }, _bottom{ _top + size._height } {}
		explicit constexpr Rect(const Size& size) noexcept
			: _right{ size._width }, _bottom{ size._height } {}

		[[nodiscard]] constexpr int bottom() const noexcept { return _bottom; }
		[[nodiscard]] constexpr Point bottomLeft() const noexcept { return { _left, _bottom }; }
		[[nodiscard]] constexpr Point bottomRight() const noexcept { return { _right, _bottom }; }
		[[nodiscard]] constexpr Point bound(const Point&) const noexcept;
		[[nodiscard]] constexpr Point center() const noexcept { return { (_left + _right) / 2, (_top + _bottom) / 2 }; }
		[[nodiscard]] constexpr Rect centeredAt(const Rect&) const noexcept;
		[[nodiscard]] constexpr bool contains(const Point&) const noexcept;
		[[nodiscard]] constexpr bool contains(const Rect&) const noexcept;
		[[nodiscard]] constexpr int height() const noexcept { return _bottom - _top; }
		[[nodiscard]] constexpr Rect intersected(const Rect&) const noexcept;
		[[nodiscard]] constexpr bool intersects(const Rect&) const noexcept;
		[[nodiscard]] constexpr bool isEmpty() const noexcept { return _left >= _right || _top >= _bottom; }
		[[nodiscard]] constexpr int left() const noexcept { return _left; }
		[[nodiscard]] constexpr int right() const noexcept { return _right; }
		[[nodiscard]] constexpr Size size() const noexcept { return { width(), height() }; }
		[[nodiscard]] constexpr int top() const noexcept { return _top; }
		[[nodiscard]] constexpr Point topLeft() const noexcept { return { _left, _top }; }
		[[nodiscard]] constexpr Point topRight() const noexcept { return { _right, _top }; }
		[[nodiscard]] constexpr int width() const noexcept { return _right - _left; }

	private:
		int _left = 0;
		int _top = 0;
		int _right = 0;
		int _bottom = 0;
	};

	[[nodiscard]] constexpr bool operator==(const Point& a, const Point& b) noexcept { return a._x == b._x && a._y == b._y; }
	[[nodiscard]] constexpr bool operator==(const Size& a, const Size& b) noexcept { return a._width == b._width && a._height == b._height; }
	[[nodiscard]] constexpr bool operator==(const Rect& a, const Rect& b) noexcept { return a.left() == b.left() && a.top() == b.top() && a.right() == b.right() && a.bottom() == b.bottom(); }
}

constexpr seir::Point seir::Rect::bound(const Point& p) const noexcept
{
	auto x = p._x;
	if (x < _left)
		x = _left;
	else if (x >= _right)
		x = _right - 1;
	auto y = p._y;
	if (y < _top)
		y = _top;
	else if (y >= _bottom)
		y = _bottom - 1;
	return { x, y };
}

constexpr seir::Rect seir::Rect::centeredAt(const Rect& r) const noexcept
{
	return {
		{ (r._right + r._left - width()) / 2, (r._bottom + r._top - height()) / 2 },
		size()
	};
}

constexpr bool seir::Rect::contains(const Point& p) const noexcept
{
	// _left <= p._x && p._x < _right && _top <= p._y && p._y < _bottom
	return ((p._x - _left) ^ (p._x - _right)) < 0
		&& ((p._y - _top) ^ (p._y - _bottom)) < 0;
}

constexpr bool seir::Rect::contains(const Rect& r) const noexcept
{
	return _left <= r._left && r._right <= _right
		&& _top <= r._top && r._bottom <= _bottom;
}

constexpr seir::Rect seir::Rect::intersected(const Rect& r) const noexcept
{
	return {
		{ _left > r._left ? _left : r._left,
			_top > r._top ? _top : r._top },
		Point{
			_right < r._right ? _right : r._right,
			_bottom < r._bottom ? _bottom : r._bottom }
	};
}

constexpr bool seir::Rect::intersects(const Rect& r) const noexcept
{
	// _left < r._right && r._left < _right && _top < r._bottom && r._top < _bottom
	return ((_left - r._right) ^ (r._left - _right)) >= 0
		&& ((_top - r._bottom) ^ (r._top - _bottom)) >= 0;
}
