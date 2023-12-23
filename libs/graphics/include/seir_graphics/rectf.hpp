// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_graphics/rect.hpp>
#include <seir_graphics/sizef.hpp>
#include <seir_math/vec.hpp>

namespace seir
{
	class RectF
	{
	public:
		float _left = 0;
		float _top = 0;
		float _right = 0;
		float _bottom = 0;

		constexpr RectF() noexcept = default;
		constexpr RectF(const Vec2& topLeft, const Vec2& bottomRight) noexcept
			: _left{ topLeft.x }, _top{ topLeft.y }, _right{ bottomRight.x }, _bottom{ bottomRight.y } {}
		constexpr RectF(const Vec2& topLeft, const SizeF& size) noexcept
			: _left{ topLeft.x }, _top{ topLeft.y }, _right{ _left + size._width }, _bottom{ _top + size._height } {}
		explicit constexpr RectF(const SizeF& size) noexcept
			: _right{ size._width }, _bottom{ size._height } {}
		explicit constexpr RectF(const Rect& rect) noexcept
			: _left{ static_cast<float>(rect.left()) }, _top{ static_cast<float>(rect.top()) }, _right{ static_cast<float>(rect.right()) }, _bottom{ static_cast<float>(rect.bottom()) } {}

		[[nodiscard]] constexpr float bottom() const noexcept { return _bottom; }
		[[nodiscard]] constexpr Vec2 bottomLeft() const noexcept { return { _left, _bottom }; }
		[[nodiscard]] constexpr Vec2 bottomRight() const noexcept { return { _right, _bottom }; }
		[[nodiscard]] constexpr Vec2 bound(const Vec2&) const noexcept;
		[[nodiscard]] constexpr Vec2 center() const noexcept { return { (_left + _right) / 2, (_top + _bottom) / 2 }; }
		[[nodiscard]] constexpr RectF centeredAt(const RectF&) const noexcept;
		[[nodiscard]] constexpr bool contains(const Vec2&) const noexcept;
		[[nodiscard]] constexpr bool contains(const RectF&) const noexcept;
		[[nodiscard]] constexpr float height() const noexcept { return _bottom - _top; }
		[[nodiscard]] constexpr RectF intersected(const RectF&) const noexcept;
		[[nodiscard]] constexpr bool intersects(const RectF&) const noexcept;
		[[nodiscard]] constexpr bool isEmpty() const noexcept { return _left >= _right || _top >= _bottom; }
		[[nodiscard]] constexpr bool isNull() const noexcept { return _left == 0 && _top == 0 && _right == 0 && _bottom == 0; } // TODO: Review usage.
		[[nodiscard]] constexpr float left() const noexcept { return _left; }
		[[nodiscard]] constexpr float right() const noexcept { return _right; }
		constexpr void setHeight(float height) noexcept { _bottom = _top + height; }
		constexpr void setWidth(float width) noexcept { _right = _left + width; }
		[[nodiscard]] constexpr SizeF size() const noexcept { return { width(), height() }; }
		[[nodiscard]] constexpr float top() const noexcept { return _top; }
		[[nodiscard]] constexpr Vec2 topLeft() const noexcept { return { _left, _top }; }
		[[nodiscard]] constexpr Vec2 topRight() const noexcept { return { _right, _top }; }
		[[nodiscard]] constexpr float width() const noexcept { return _right - _left; }
	};

	[[nodiscard]] constexpr RectF operator+(const RectF&, const Vec2&) noexcept;

	[[nodiscard]] constexpr RectF operator-(const RectF&, const Vec2&) noexcept;

	[[nodiscard]] constexpr RectF operator*(const RectF&, float) noexcept;
	[[nodiscard]] constexpr RectF operator*(const RectF&, const SizeF&) noexcept;

	[[nodiscard]] constexpr RectF operator/(const RectF&, float) noexcept;
	[[nodiscard]] constexpr RectF operator/(const RectF&, const SizeF&) noexcept;
}

constexpr seir::Vec2 seir::RectF::bound(const Vec2& p) const noexcept
{
	auto x = p.x;
	if (x < _left)
		x = _left;
	else if (x >= _right)
		x = _right - 1;
	auto y = p.y;
	if (y < _top)
		y = _top;
	else if (y >= _bottom)
		y = _bottom - 1;
	return { x, y };
}

constexpr seir::RectF seir::RectF::centeredAt(const RectF& r) const noexcept
{
	return {
		{ (r._right + r._left - width()) / 2, (r._bottom + r._top - height()) / 2 },
		size()
	};
}

constexpr bool seir::RectF::contains(const Vec2& p) const noexcept
{
	return _left <= p.x && p.x < _right
		&& _top <= p.y && p.y < _bottom;
}

constexpr bool seir::RectF::contains(const RectF& r) const noexcept
{
	return _left <= r._left && r._right <= _right
		&& _top <= r._top && r._bottom <= _bottom;
}

constexpr seir::RectF seir::RectF::intersected(const RectF& r) const noexcept
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

constexpr bool seir::RectF::intersects(const RectF& r) const noexcept
{
	return _left < r._right && r._left < _right
		&& _top < r._bottom && r._top < _bottom;
}

constexpr seir::RectF seir::operator+(const RectF& a, const Vec2& b) noexcept
{
	return { a.topLeft() + b, a.bottomRight() + b };
}

constexpr seir::RectF seir::operator-(const RectF& a, const Vec2& b) noexcept
{
	return { a.topLeft() - b, a.bottomRight() - b };
}

constexpr seir::RectF seir::operator*(const RectF& a, float b) noexcept
{
	return { a.topLeft() * b, a.bottomRight() * b };
}

constexpr seir::RectF seir::operator*(const RectF& a, const SizeF& b) noexcept
{
	return {
		{ a.left() * b._width, a.top() * b._width },
		Vec2{ a.right() * b._width, a.bottom() * b._width }
	};
}

constexpr seir::RectF seir::operator/(const RectF& a, float b) noexcept
{
	return { a.topLeft() / b, a.bottomRight() / b };
}

constexpr seir::RectF seir::operator/(const RectF& a, const SizeF& b) noexcept
{
	return {
		{ a.left() / b._width, a.top() / b._width },
		Vec2{ a.right() / b._width, a.bottom() / b._width }
	};
}
