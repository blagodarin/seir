// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_renderer/renderer.hpp>

#include <seir_image/image.hpp>

namespace seir
{
	SharedPtr<Texture2D> Renderer::createTexture2D(const Image& image)
	{
		return createTexture2D(image.info(), image.data());
	}
}
