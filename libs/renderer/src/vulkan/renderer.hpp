// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_renderer/renderer.hpp>
#include "context.hpp"
#include "pipeline.hpp"

namespace seir
{
	class VulkanRenderer final : public Renderer
	{
	public:
		explicit VulkanRenderer(const SharedPtr<Window>&) noexcept;
		~VulkanRenderer() noexcept override;

		bool initialize();

		void draw() override;

	private:
		const SharedPtr<Window> _window;
		VulkanContext _context;
		VulkanShader _vertexShader;
		VulkanShader _fragmentShader;
		VulkanImage _textureImage;
		VulkanSampler _textureSampler;
		VulkanBuffer _vertexBuffer;
		VulkanBuffer _indexBuffer;
		VulkanFrameSync _frameSync;
		VulkanRenderTarget _renderTarget;
		VulkanPipeline _pipeline;
		VulkanUniformBuffers _uniformBuffers;
		std::vector<VulkanDescriptorAllocator> _descriptorAllocators;
	};
}
