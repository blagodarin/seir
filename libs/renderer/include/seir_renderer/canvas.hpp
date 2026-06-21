// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <memory>

namespace seir
{
	class Quad;
	class Rect;
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
		void drawQuad(const Quad&);

		//
		void drawRect(const Rect&);

		// Submits the canvas for actual rendering.
		// The canvas content is reset after submission.
		void render(RenderPass&);

		//
		void setColor(const Rgba32&);

		//
		void setTexture(const SharedPtr<Texture2D>&);

		//
		void setTextureRect(const Rect&);

	private:
		const std::unique_ptr<class CanvasImpl> _impl;
	};
}
