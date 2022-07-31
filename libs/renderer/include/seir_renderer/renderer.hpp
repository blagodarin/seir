// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/shared_ptr.hpp>

namespace seir
{
	class Image;
	class ImageInfo;
	class Window;

	//
	class Texture2D : public ReferenceCounter
	{
	};

	//
	class Renderer
	{
	public:
		//
		[[nodiscard]] static UniquePtr<Renderer> create(const SharedPtr<Window>&);

		virtual ~Renderer() noexcept = default;

		//
		[[nodiscard]] virtual UniquePtr<Texture2D> createTexture2D(const ImageInfo&, const void*) = 0;
		[[nodiscard]] UniquePtr<Texture2D> createTexture2D(const Image&);

		//
		virtual void draw() = 0;
	};
}
