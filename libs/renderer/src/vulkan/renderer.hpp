// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_renderer/renderer.hpp>
#include "2d.hpp"
#include "context.hpp"
#include "descriptors.hpp"
#include "pipeline.hpp"

namespace seir
{
	class VulkanRenderPass;
	class VulkanShaderSet;
	class VulkanTexture2D;

	class RendererImpl
	{
	public:
		explicit RendererImpl(const Window&) noexcept;
		~RendererImpl() noexcept;

		SharedPtr<ShaderSet> createShaders(std::span<const uint32_t>, std::span<const uint32_t>);

	private:
		bool initialize();
		void render(const std::function<void(RenderPass&)>&);
		void resetRenderTarget();

		static std::unique_ptr<RendererImpl> create(const Window&);

	private:
		const Window& _window;
		VulkanContext _context;
		VulkanSampler _textureSampler;
		VulkanFrameSync _frameSync;
		SharedPtr<VulkanTexture2D> _whiteTexture2D;
		VulkanRenderTarget _renderTarget;
		std::unordered_multimap<const VulkanShaderSet*, std::pair<unsigned, VulkanPipeline>> _pipelineCache;
		VulkanUniformBuffers _uniformBuffers;
		vulkan::DescriptorAllocator _descriptorAllocator;
		Vulkan2D _2d;
		friend Renderer;
		friend VulkanRenderPass;
	};
}
