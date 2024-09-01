// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_gui/layout.hpp>

#include <seir_gui/frame.hpp>
#include "context_impl.hpp"

namespace seir
{
	GuiLayout::GuiLayout(GuiFrame& frame) noexcept
		: _frame{ frame }
		, _previous{ std::exchange(frame._context._layout, this) }
		, _size{ frame._size }
	{
	}

	GuiLayout::GuiLayout(GuiFrame& frame, const Center& mapping) noexcept
		: _frame{ frame }
		, _previous{ std::exchange(frame._context._layout, this) }
		, _size{ mapping._width, mapping._height }
	{
		const auto widthRatio = frame._size._width / mapping._width;
		const auto heightRatio = frame._size._height / mapping._height;
		if (widthRatio > heightRatio)
		{
			_scaling = heightRatio;
			_offset = { (frame._size._width - mapping._width * _scaling) / 2, 0 };
		}
		else
		{
			_scaling = widthRatio;
			_offset = { 0, (frame._size._height - mapping._height * _scaling) / 2 };
		}
	}

	GuiLayout::GuiLayout(GuiFrame& frame, const Height& mapping) noexcept
		: _frame{ frame }
		, _previous{ std::exchange(frame._context._layout, this) }
		, _scaling{ frame._size._height / mapping._height }
		, _size{ frame._size._width / _scaling, mapping._height }
	{
	}

	GuiLayout::GuiLayout(GuiFrame& frame, const Width& mapping) noexcept
		: _frame{ frame }
		, _previous{ std::exchange(frame._context._layout, this) }
		, _scaling{ frame._size._width / mapping._width }
		, _size{ mapping._width, frame._size._height / _scaling }
	{
	}

	GuiLayout::~GuiLayout() noexcept
	{
		_frame._context._layout = _previous;
	}

	RectF GuiLayout::addItem(const SizeF& size) noexcept
	{
		const auto x1 = _position.x + size._width * (_direction.x - 1) / 2;
		const auto x2 = _position.x + size._width * (_direction.x + 1) / 2;
		const auto y1 = _position.y + size._height * (_direction.y - 1) / 2;
		const auto y2 = _position.y + size._height * (_direction.y + 1) / 2;
		if (_axis == Axis::X)
			_position.x = _direction.x > 0 ? x2 + _spacing : x1 - _spacing;
		else
			_position.y = _direction.y > 0 ? y2 + _spacing : y1 - _spacing;
		return RectF{ { x1, y1 }, Vec2{ x2, y2 } } * _scaling + _offset;
	}

	void GuiLayout::fromPoint(const Vec2& point, const Vec2& direction, Axis axis, float padding) noexcept
	{
		_direction = direction;
		_position = point + padding * _direction;
		_axis = axis;
	}

	RectF GuiLayout::map(const RectF& rect) const noexcept
	{
		return rect * _scaling + _offset;
	}

	void GuiLayout::skip(float distance) noexcept
	{
		if (_axis == Axis::X)
			_position.x += _direction.x * distance;
		else
			_position.y += _direction.y * distance;
	}
}
