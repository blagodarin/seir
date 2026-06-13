// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <memory>

namespace seir
{
	class QuadF;
	class RectF;
	class RenderPass;
	class Rgba32;
	template <class>
	class SharedPtr;
	class Texture2D;

	//
	class Canvas
	{
	public:
		Canvas();
		~Canvas() noexcept;

		//
		void drawQuad(const QuadF&);

		//
		void drawRect(const RectF&);

		// Submits the canvas for actual rendering.
		// The canvas content is reset after submission.
		void render(RenderPass&);

		//
		void setColor(const Rgba32&);

		//
		void setTexture(const SharedPtr<Texture2D>&);

		//
		void setTextureRect(const RectF&);

	private:
		const std::unique_ptr<class CanvasImpl> _impl;
	};
}
