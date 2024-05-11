// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <string_view>

namespace seir
{
	class Blob;
	class RectF;
	class Renderer;
	class Renderer2D;
	template <class>
	class SharedPtr;
	class Texture2D;
	template <class>
	class UniquePtr;

	//
	class Font
	{
	public:
		//
		[[nodiscard]] static UniquePtr<Font> load(const SharedPtr<Blob>&, unsigned lineHeight, Renderer&);

		virtual ~Font() noexcept = default;

		//
		[[nodiscard]] virtual SharedPtr<Texture2D> bitmapTexture() const noexcept = 0;

		//
		virtual void renderLine(Renderer2D&, const RectF&, std::string_view) const = 0;

		//
		[[nodiscard]] virtual float size() const noexcept = 0;

		//
		[[nodiscard]] virtual float textWidth(std::string_view text, float fontSize) const noexcept = 0;

		//
		[[nodiscard]] virtual RectF whiteRect() const noexcept = 0;
	};
}
