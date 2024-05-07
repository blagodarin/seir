// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_gui/font.hpp>

#include <seir_data/blob.hpp>

#include <limits>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace
{
	constexpr FT_UInt kPixelSize = 64;

	class FreeTypeFont : public seir::Font
	{
	public:
		FreeTypeFont(const seir::SharedPtr<seir::Blob>& blob)
		{
			if (::FT_Init_FreeType(&_library))
				return;
			if (blob->size() > static_cast<size_t>(std::numeric_limits<FT_Long>::max())
				|| ::FT_New_Memory_Face(_library, static_cast<const FT_Byte*>(blob->data()), static_cast<FT_Long>(blob->size()), 0, &_face))
				return;
			_blob = blob;
			::FT_Set_Pixel_Sizes(_face, 0, kPixelSize);
			_hasKerning = FT_HAS_KERNING(_face);
		}

		~FreeTypeFont() noexcept override
		{
			if (_face)
				::FT_Done_Face(_face);    // TODO: Handle error code.
			::FT_Done_FreeType(_library); // TODO: Handle error code.
		}

		bool isLoaded() noexcept
		{
			return static_cast<bool>(_blob);
		}

	private:
		FT_Library _library = nullptr;
		seir::SharedPtr<seir::Blob> _blob;
		FT_Face _face = nullptr;
		bool _hasKerning = false;
	};
}

namespace seir
{
	UniquePtr<Font> Font::load(const SharedPtr<Blob>& blob)
	{
		if (!blob)
			return {};
		auto font = makeUnique<FreeTypeFont>(blob);
		return font->isLoaded()
			? staticCast<Font>(std::move(font))
			: nullptr;
	}
}
