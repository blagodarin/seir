// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_renderer/renderer.hpp>

#include "canvas.hpp"

namespace seir
{
	class RenderPassImpl : public RenderPass
	{
	public:
		virtual void beginCanvasRendering(const MeshFormat&) = 0;
		virtual void bindCanvasShaders() = 0;
		virtual void renderCanvasRange(uint32_t firstIndex, uint32_t indexCount) = 0;
		virtual void updateCanvasBuffers(std::span<const CanvasVertex> vertices, std::span<const uint16_t> indices) = 0;
	};
}
