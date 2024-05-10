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
		[[nodiscard]] static UniquePtr<Font> load(const SharedPtr<Blob>&, Renderer&);

		virtual ~Font() noexcept = default;

		//
		[[nodiscard]] virtual SharedPtr<const Texture2D> bitmapTexture() const noexcept = 0;

		//
		[[nodiscard]] virtual float size() const noexcept = 0;

		//
		[[nodiscard]] virtual float textWidth(std::string_view text, float fontSize) const noexcept = 0;

		//
		[[nodiscard]] virtual RectF whiteRect() const noexcept = 0;
	};
}
