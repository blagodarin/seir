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
	class VulkanTexture2D;

	class VulkanRenderer final : public Renderer
	{
	public:
		explicit VulkanRenderer(const SharedPtr<Window>&) noexcept;
		~VulkanRenderer() noexcept override;

		bool initialize();

		UniquePtr<Mesh> createMesh(const MeshFormat&, const void*, size_t, const void*, size_t) override;
		SharedPtr<Texture2D> createTexture2D(const ImageInfo&, const void*) override;
		void render(const std::function<void(const Vec2&, RenderPass&)>&) override;

	private:
		void resetRenderTarget();

	private:
		const SharedPtr<Window> _window;
		VulkanContext _context;
		VulkanShader _vertexShader;
		VulkanShader _fragmentShader;
		VulkanSampler _textureSampler;
		VulkanFrameSync _frameSync;
		SharedPtr<VulkanTexture2D> _whiteTexture2D;
		VulkanRenderTarget _renderTarget;
		std::unordered_map<unsigned, VulkanPipeline> _pipelineCache;
		VulkanUniformBuffers _uniformBuffers;
		vulkan::DescriptorAllocator _descriptorAllocator;
		friend VulkanRenderPass;
	};
}
