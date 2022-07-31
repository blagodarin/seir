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
	class VulkanRenderer final : public Renderer
	{
	public:
		explicit VulkanRenderer(const SharedPtr<Window>&) noexcept;
		~VulkanRenderer() noexcept override;

		bool initialize();

		UniquePtr<Texture2D> createTexture2D(const ImageInfo&, const void*) override;
		void draw() override;

	private:
		void resetRenderTarget();

	private:
		const SharedPtr<Window> _window;
		VulkanContext _context;
		VulkanShader _vertexShader;
		VulkanShader _fragmentShader;
		VulkanSampler _textureSampler;
		VulkanBuffer _vertexBuffer;
		VulkanBuffer _indexBuffer;
		VulkanFrameSync _frameSync;
		UniquePtr<Texture2D> _whiteTexture2D;
		VulkanRenderTarget _renderTarget;
		VulkanPipeline _pipeline;
		VulkanUniformBuffers _uniformBuffers;
		vulkan::DescriptorAllocator _descriptorAllocator;
	};
}
