// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_renderer/renderer.hpp>
#include "context.hpp"
#include "descriptors.hpp"
#include "pipeline.hpp"

namespace seir
{
	class VulkanRenderPass;
	class VulkanShaderSet;
	class VulkanTexture2D;

	class VulkanRenderer final : public Renderer
	{
	public:
		explicit VulkanRenderer(const SharedPtr<Window>&) noexcept;
		~VulkanRenderer() noexcept override;

		bool initialize();

		UniquePtr<Mesh> createMesh(const MeshFormat&, const void*, size_t, const void*, size_t) override;
		SharedPtr<ShaderSet> createShaders(std::span<const uint32_t>, std::span<const uint32_t>) override;
		SharedPtr<Texture2D> createTexture2D(const ImageInfo&, const void*) override;
		void render(const std::function<Mat4(const Vec2&)>&, const std::function<void(RenderPass&)>&) override;

	private:
		void resetRenderTarget();

	private:
		const SharedPtr<Window> _window;
		VulkanContext _context;
		VulkanSampler _textureSampler;
		VulkanFrameSync _frameSync;
		SharedPtr<VulkanTexture2D> _whiteTexture2D;
		VulkanRenderTarget _renderTarget;
		std::unordered_multimap<const VulkanShaderSet*, std::pair<unsigned, VulkanPipeline>> _pipelineCache;
		VulkanUniformBuffers _uniformBuffers;
		vulkan::DescriptorAllocator _descriptorAllocator;
		friend VulkanRenderPass;
	};
}
