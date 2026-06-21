// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_math/size.hpp>
#include <seir_math/vec.hpp>

namespace seir
{
	// Axis-aligned 2D rectangle.
	// Assumes X is left to right and Y is top to bottom.
	// A rectangle contains its top and left sides, but doesn't contain bottom and right sides.
	class Rect
	{
	public:
		float _left = 0;
		float _top = 0;
		float _right = 0;
		float _bottom = 0;

		constexpr Rect() noexcept = default;
		constexpr Rect(const Vec2& topLeft, const Vec2& bottomRight) noexcept
			: _left{ topLeft.x }, _top{ topLeft.y }, _right{ bottomRight.x }, _bottom{ bottomRight.y } {}
		constexpr Rect(const Vec2& topLeft, const Size2D& size) noexcept
			: _left{ topLeft.x }, _top{ topLeft.y }, _right{ _left + size.width }, _bottom{ _top + size.height } {}
		explicit constexpr Rect(const Size2D& size) noexcept
			: _right{ size.width }, _bottom{ size.height } {}

		[[nodiscard]] constexpr float bottom() const noexcept { return _bottom; }
		[[nodiscard]] constexpr Vec2 bottomLeft() const noexcept { return { _left, _bottom }; }
		[[nodiscard]] constexpr Vec2 bottomRight() const noexcept { return { _right, _bottom }; }
		[[nodiscard]] constexpr Vec2 bound(const Vec2&) const noexcept;
		[[nodiscard]] constexpr Vec2 center() const noexcept { return { (_left + _right) / 2, (_top + _bottom) / 2 }; }
		[[nodiscard]] constexpr Rect centeredAt(const Rect&) const noexcept;
		[[nodiscard]] constexpr bool contains(const Vec2&) const noexcept;
		[[nodiscard]] constexpr bool contains(const Rect&) const noexcept;
		[[nodiscard]] constexpr float height() const noexcept { return _bottom - _top; }
		[[nodiscard]] constexpr Rect intersection(const Rect&) const noexcept;
		[[nodiscard]] constexpr bool intersects(const Rect&) const noexcept;
		[[nodiscard]] constexpr bool isEmpty() const noexcept { return _left >= _right || _top >= _bottom; }
		[[nodiscard]] constexpr float left() const noexcept { return _left; }
		[[nodiscard]] constexpr float right() const noexcept { return _right; }
		constexpr void setHeight(float height) noexcept { _bottom = _top + height; }
		constexpr void setWidth(float width) noexcept { _right = _left + width; }
		[[nodiscard]] constexpr Size2D size() const noexcept { return { width(), height() }; }
		[[nodiscard]] constexpr float top() const noexcept { return _top; }
		[[nodiscard]] constexpr Vec2 topLeft() const noexcept { return { _left, _top }; }
		[[nodiscard]] constexpr Vec2 topRight() const noexcept { return { _right, _top }; }
		[[nodiscard]] constexpr float width() const noexcept { return _right - _left; }
	};

	[[nodiscard]] constexpr Rect operator+(const Rect&, const Vec2&) noexcept;

	[[nodiscard]] constexpr Rect operator-(const Rect&, const Vec2&) noexcept;

	[[nodiscard]] constexpr Rect operator*(const Rect&, float) noexcept;
	[[nodiscard]] constexpr Rect operator*(const Rect&, const Size2D&) noexcept;

	[[nodiscard]] constexpr Rect operator/(const Rect&, float) noexcept;
	[[nodiscard]] constexpr Rect operator/(const Rect&, const Size2D&) noexcept;
}

constexpr seir::Vec2 seir::Rect::bound(const Vec2& p) const noexcept
{
	// The current algorithm bounds points to the right and bottom sides
	// which are considered to be outside the rectangle.
	// No idea though whether this behavior should be fixed.
	auto x = p.x;
	if (x < _left)
		x = _left;
	else if (x >= _right)
		x = _right; // std::nextafter(x, _left)?
	auto y = p.y;
	if (y < _top)
		y = _top;
	else if (y >= _bottom)
		y = _bottom; // std::nextafter(y, _top)?
	return { x, y };
}

constexpr seir::Rect seir::Rect::centeredAt(const Rect& r) const noexcept
{
	return {
		{ (r._right + r._left - width()) / 2, (r._bottom + r._top - height()) / 2 },
		size()
	};
}

constexpr bool seir::Rect::contains(const Vec2& p) const noexcept
{
	return _left <= p.x && p.x < _right
		&& _top <= p.y && p.y < _bottom;
}

constexpr bool seir::Rect::contains(const Rect& r) const noexcept
{
	return _left <= r._left && r._right <= _right
		&& _top <= r._top && r._bottom <= _bottom;
}

constexpr seir::Rect seir::Rect::intersection(const Rect& r) const noexcept
{
	return {
		{
			_left > r._left ? _left : r._left,
			_top > r._top ? _top : r._top,
		},
		Vec2{
			_right < r._right ? _right : r._right,
			_bottom < r._bottom ? _bottom : r._bottom,
		}
	};
}

constexpr bool seir::Rect::intersects(const Rect& r) const noexcept
{
	return _left < r._right && r._left < _right
		&& _top < r._bottom && r._top < _bottom;
}

constexpr seir::Rect seir::operator+(const Rect& a, const Vec2& b) noexcept
{
	return { a.topLeft() + b, a.bottomRight() + b };
}

constexpr seir::Rect seir::operator-(const Rect& a, const Vec2& b) noexcept
{
	return { a.topLeft() - b, a.bottomRight() - b };
}

constexpr seir::Rect seir::operator*(const Rect& a, float b) noexcept
{
	return { a.topLeft() * b, a.bottomRight() * b };
}

constexpr seir::Rect seir::operator*(const Rect& a, const Size2D& b) noexcept
{
	return {
		{ a.left() * b.width, a.top() * b.height },
		Vec2{ a.right() * b.width, a.bottom() * b.height }
	};
}

constexpr seir::Rect seir::operator/(const Rect& a, float b) noexcept
{
	return { a.topLeft() / b, a.bottomRight() / b };
}

constexpr seir::Rect seir::operator/(const Rect& a, const Size2D& b) noexcept
{
	return {
		{ a.left() / b.width, a.top() / b.height },
		Vec2{ a.right() / b.width, a.bottom() / b.height }
	};
}
