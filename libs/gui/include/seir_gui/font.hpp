// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

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
	};
}
