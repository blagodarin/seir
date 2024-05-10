// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_renderer/2d.hpp>

#include "2d.hpp"

namespace seir
{
	class Renderer2DImpl
	{
	};

	Renderer2D::Renderer2D()
		: _impl{ std::make_unique<Renderer2DImpl>() }
	{
	}

	Renderer2D ::~Renderer2D() noexcept = default;

	void Renderer2D::addRect(const RectF&)
	{
	}

	void Renderer2D::setColor(const Rgba32&)
	{
	}

	void Renderer2D::setTexture(const SharedPtr<Texture2D>&)
	{
	}

	void Renderer2D::setTextureRect(const RectF&)
	{
	}
}
