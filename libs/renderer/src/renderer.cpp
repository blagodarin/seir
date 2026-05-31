// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_renderer/renderer.hpp>

#include <seir_image/image.hpp>
#include <seir_model/mesh_data.hpp>

namespace seir
{
	SharedPtr<Mesh> Renderer::createMesh(const SharedPtr<MeshData>& data)
	{
		if (!data) [[unlikely]]
			return {};
		return createMesh(data->format(), data->vertexData(), data->vertexCount(), data->indexData(), data->indexCount());
	}

	SharedPtr<Texture2D> Renderer::createTexture2D(const Image& image)
	{
		return createTexture2D(image.info(), image.data());
	}
}
