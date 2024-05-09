// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <string_view>

namespace seir
{
	class Blob;
	template <class>
	class SharedPtr;
	template <class>
	class UniquePtr;

	//
	class Font
	{
	public:
		//
		[[nodiscard]] static UniquePtr<Font> load(const SharedPtr<Blob>&);

		virtual ~Font() noexcept = default;

		//
		[[nodiscard]] virtual float size() const noexcept = 0;

		//
		[[nodiscard]] virtual float textWidth(std::string_view text, float fontSize) const noexcept = 0;
	};
}
