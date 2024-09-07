// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_gui/font.hpp>

#include <seir_base/utf8.hpp>
#include <seir_data/blob.hpp>
#include <seir_graphics/rectf.hpp>
#include <seir_image/image.hpp>
#include <seir_image/utils.hpp>
#include <seir_renderer/2d.hpp>
#include <seir_renderer/renderer.hpp>

#include <cassert>
#include <cstring>
#include <limits>
#include <unordered_map>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace
{
	constexpr size_t kWhiteWidth = 4;
	constexpr size_t kWhiteHeight = 4;
	constexpr uint8_t kWhiteData[kWhiteHeight][kWhiteWidth]{
		{ 0xff, 0xff, 0x00, 0x00 },
		{ 0xff, 0xff, 0x00, 0x00 },
		{ 0x00, 0x00, 0x00, 0x00 },
		{ 0x00, 0x00, 0x00, 0x00 },
	};
	constexpr seir::RectF kWhiteRect{ {}, seir::SizeF{ 1, 1 } };

	class FreeTypeFont final : public seir::Font
	{
	public:
		FreeTypeFont(seir::Renderer& renderer, const seir::SharedPtr<seir::Blob>& blob, unsigned lineHeight)
		{
			if (::FT_Init_FreeType(&_library))
				return;
			if (blob->size() > static_cast<size_t>(std::numeric_limits<FT_Long>::max())
				|| ::FT_New_Memory_Face(_library, static_cast<const FT_Byte*>(blob->data()), static_cast<FT_Long>(blob->size()), 0, &_face))
				return;
			_blob = blob;
			_hasKerning = FT_HAS_KERNING(_face);
			buildBitmap(renderer, lineHeight);
		}

		~FreeTypeFont() noexcept override
		{
			if (_face)
				::FT_Done_Face(_face);    // TODO: Handle error code.
			::FT_Done_FreeType(_library); // TODO: Handle error code.
		}

		bool isLoaded() noexcept
		{
			return static_cast<bool>(_bitmapTexture);
		}

		seir::SharedPtr<seir::Texture2D> bitmapTexture() const noexcept override
		{
			return _bitmapTexture;
		}

		void renderLine(seir::Renderer2D& renderer, const seir::RectF& rect, std::string_view text) const override
		{
			const auto scale = rect.height() / static_cast<float>(_size);
			int x = 0;
			auto previous = _bitmapGlyphs.end();
			renderer.setTexture(_bitmapTexture);
			for (size_t i = 0; i < text.size();)
			{
				const auto current = _bitmapGlyphs.find(seir::readUtf8(text, i));
				if (current == _bitmapGlyphs.end())
					continue;
				if (_hasKerning && previous != _bitmapGlyphs.end())
				{
					FT_Vector kerning;
					if (!::FT_Get_Kerning(_face, previous->second._id, current->second._id, FT_KERNING_DEFAULT, &kerning))
						x += static_cast<int>(kerning.x >> 6);
				}
				const auto left = rect.left() + static_cast<float>(x + current->second._offset._x) * scale;
				if (left >= rect.right())
					break;
				seir::RectF positionRect{
					{ left, rect.top() + static_cast<float>(current->second._offset._y) * scale },
					seir::SizeF{ current->second._rect.size() } * scale,
				};
				seir::RectF glyphRect{ current->second._rect };
				bool clipped = false;
				if (positionRect.right() > rect.right())
				{
					const auto originalWidth = positionRect.width();
					positionRect._right = rect._right;
					glyphRect.setWidth(glyphRect.width() * positionRect.width() / originalWidth);
					clipped = true;
				}
				renderer.setTextureRect(glyphRect);
				renderer.addRect(positionRect);
				if (clipped)
					break;
				x += current->second._advance;
				previous = current;
			}
		}

		float size() const noexcept override
		{
			return _size;
		}

		float textWidth(std::string_view text, float fontSize, TextCapture* capture) const noexcept override
		{
			const auto scale = fontSize / _size;
			int x = 0;
			std::optional<float> selectionX;
			const auto updateCapture = [&](size_t offset) {
				if (!capture)
					return;
				if (capture->_cursorOffset == offset)
					capture->_cursorPosition.emplace(static_cast<float>(x) * scale);
				if (capture->_selectionBegin < capture->_selectionEnd)
				{
					if (selectionX)
					{
						if (offset == capture->_selectionEnd)
						{
							capture->_selectionRange.emplace(*selectionX, static_cast<float>(x) * scale);
							selectionX.reset();
						}
					}
					else if (offset == capture->_selectionBegin)
						selectionX = static_cast<float>(x) * scale;
				}
			};
			auto previous = _bitmapGlyphs.end();
			for (size_t i = 0; i < text.size();)
			{
				const auto offset = i;
				const auto current = _bitmapGlyphs.find(seir::readUtf8(text, i));
				if (current == _bitmapGlyphs.end())
					continue;
				if (_hasKerning && previous != _bitmapGlyphs.end())
				{
					FT_Vector kerning;
					if (!::FT_Get_Kerning(_face, previous->second._id, current->second._id, FT_KERNING_DEFAULT, &kerning))
						x += static_cast<int>(kerning.x >> 6);
				}
				updateCapture(offset);
				x += current->second._advance;
				previous = current;
			}
			updateCapture(text.size());
			return static_cast<float>(x) * scale;
		}

		seir::RectF whiteRect() const noexcept override
		{
			return ::kWhiteRect;
		}

	private:
		void buildBitmap(seir::Renderer& renderer, FT_UInt size)
		{
			assert(_bitmapGlyphs.empty());
			_size = static_cast<float>(size);
			const seir::ImageInfo bitmapInfo{ size * 32, size * 32, seir::PixelFormat::Intensity8 };
			seir::Buffer bitmapBuffer{ bitmapInfo.frameSize() };
			std::memset(bitmapBuffer.data(), 0, bitmapBuffer.capacity());
			size_t x = 0;
			size_t y = 0;
			size_t rowHeight = 0;
			const auto copyGlyph = [&](const uint8_t* src, size_t width, size_t height, ptrdiff_t stride) {
				if (height > 0)
				{
					if (stride < 0)
						src += (height - 1) * static_cast<size_t>(-stride);
					auto dst = bitmapBuffer.data() + bitmapInfo.stride() * y + x;
					for (size_t row = 0; row < height; ++row)
					{
						std::memcpy(dst, src, width);
						src += stride;
						dst += bitmapInfo.stride();
					}
					if (rowHeight < height)
						rowHeight = height;
				}
				x += width + 1;
			};
			copyGlyph(::kWhiteData[0], ::kWhiteWidth, ::kWhiteHeight, ::kWhiteWidth);
			::FT_Set_Pixel_Sizes(_face, 0, size);
			const auto baseline = static_cast<FT_Int>(size) * _face->ascender / _face->height;
			for (FT_UInt codepoint = 0; codepoint < 65536; ++codepoint)
			{
				const auto id = FT_Get_Char_Index(_face, codepoint);
				if (!id)
					continue;
				if (FT_Load_Glyph(_face, id, FT_LOAD_RENDER))
					continue; // TODO: Report error.
				const auto glyph = _face->glyph;
				if (x + glyph->bitmap.width > bitmapInfo.width())
				{
					x = 0;
					y += rowHeight + 1;
					rowHeight = 0;
				}
				if (y + glyph->bitmap.rows > bitmapInfo.height())
					break; // TODO: Report error.
				auto& bitmapGlyph = _bitmapGlyphs[codepoint];
				bitmapGlyph._id = id;
				bitmapGlyph._rect = {
					{ static_cast<int>(x), static_cast<int>(y) },
					seir::Size{ static_cast<int>(glyph->bitmap.width), static_cast<int>(glyph->bitmap.rows) },
				};
				bitmapGlyph._offset = { glyph->bitmap_left, baseline - glyph->bitmap_top };
				bitmapGlyph._advance = static_cast<int>(glyph->advance.x >> 6);
				copyGlyph(glyph->bitmap.buffer, glyph->bitmap.width, glyph->bitmap.rows, glyph->bitmap.pitch);
			}
			const seir::ImageInfo textureInfo{ bitmapInfo.width(), bitmapInfo.height(), seir::PixelFormat::Bgra32 };
			seir::Buffer textureBuffer{ textureInfo.frameSize() };
			seir::copyImage(bitmapInfo, bitmapBuffer.data(), textureInfo, textureBuffer.data());
			_bitmapTexture = renderer.createTexture2D(textureInfo, textureBuffer.data());
		}

	private:
		struct Glyph
		{
			FT_UInt _id = 0;
			seir::Rect _rect;
			seir::Point _offset;
			int _advance = 0;
		};

		FT_Library _library = nullptr;
		seir::SharedPtr<seir::Blob> _blob;
		FT_Face _face = nullptr;
		bool _hasKerning = false;
		float _size = 0;
		std::unordered_map<char32_t, Glyph> _bitmapGlyphs; // TODO: Use single allocation container.
		seir::SharedPtr<seir::Texture2D> _bitmapTexture;
	};
}

namespace seir
{
	SharedPtr<Font> Font::load(Renderer& renderer, const SharedPtr<Blob>& blob, unsigned lineHeight)
	{
		if (!blob || !lineHeight)
			return {};
		const auto font = makeShared<FreeTypeFont>(renderer, blob, lineHeight);
		return font->isLoaded()
			? staticCast<Font>(font)
			: nullptr;
	}
}
