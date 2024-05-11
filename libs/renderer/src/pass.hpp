// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_renderer/renderer.hpp>

#include "2d.hpp"

namespace seir
{
	class RenderPassImpl : public RenderPass
	{
	public:
		virtual void begin2DRendering(const MeshFormat&) = 0;
		virtual void bind2DShaders() = 0;
		virtual void draw2D(uint32_t firstIndex, uint32_t indexCount) = 0;
		virtual void update2DBuffers(std::span<const Vertex2D> vertices, std::span<const uint16_t> indices) = 0;
	};
}
