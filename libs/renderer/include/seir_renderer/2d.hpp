// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <memory>

namespace seir
{
	class RectF;
	class RenderPass;
	class Rgba32;
	template <class>
	class SharedPtr;
	class Texture2D;

	//
	class Renderer2D
	{
	public:
		Renderer2D();
		~Renderer2D() noexcept;

		//
		void addRect(const RectF&);

		//
		void draw(RenderPass&);

		//
		void setColor(const Rgba32&);

		//
		void setTexture(const SharedPtr<Texture2D>&);

		//
		void setTextureRect(const RectF&);

	private:
		const std::unique_ptr<class Renderer2DImpl> _impl;
	};
}
