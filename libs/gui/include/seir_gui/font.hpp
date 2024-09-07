// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/shared_ptr.hpp>

#include <optional>
#include <string_view>

namespace seir
{
	class Blob;
	class RectF;
	class Renderer;
	class Renderer2D;
	class Texture2D;

	//
	class Font : public ReferenceCounter
	{
	public:
		struct TextCapture
		{
			const size_t _cursorOffset;
			const size_t _selectionBegin;
			const size_t _selectionEnd;
			std::optional<float> _cursorPosition;
			std::optional<std::pair<float, float>> _selectionRange;

			constexpr TextCapture(size_t cursorOffset, size_t selectionOffset, size_t selectionSize) noexcept
				: _cursorOffset{ cursorOffset }, _selectionBegin{ selectionOffset }, _selectionEnd{ selectionOffset + selectionSize } {}
		};

		//
		[[nodiscard]] static SharedPtr<Font> load(Renderer&, const SharedPtr<Blob>&, unsigned lineHeight);

		virtual ~Font() noexcept = default;

		//
		[[nodiscard]] virtual SharedPtr<Texture2D> bitmapTexture() const noexcept = 0;

		//
		virtual void renderLine(Renderer2D&, const RectF&, std::string_view) const = 0;

		//
		[[nodiscard]] virtual float size() const noexcept = 0;

		//
		virtual float textWidth(std::string_view text, float fontSize, TextCapture* = nullptr) const noexcept = 0;

		//
		[[nodiscard]] virtual RectF whiteRect() const noexcept = 0;
	};
}
