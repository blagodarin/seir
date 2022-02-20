// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/static_vector.hpp>

#include "vulkan.hpp"

namespace seir
{
	enum class VertexAttribute
	{
		f32x2,
		f32x3,
	};

	class VulkanPipelineBuilder
	{
	public:
		explicit VulkanPipelineBuilder(const VkExtent2D&) noexcept;

		VkPipeline build(VkDevice, VkPipelineLayout, VkRenderPass);
		void setInputAssembly(VkPrimitiveTopology, bool enablePrimitiveRestart) noexcept;
		void setStage(VkShaderStageFlagBits, VkShaderModule) noexcept;
		void setVertexInput(uint32_t binding, std::initializer_list<VertexAttribute>, VkVertexInputRate = VK_VERTEX_INPUT_RATE_VERTEX) noexcept;

	private:
		StaticVector<VkPipelineShaderStageCreateInfo, 4> _stages;
		StaticVector<VkVertexInputBindingDescription, 1> _vertexInputBindings;
		StaticVector<VkVertexInputAttributeDescription, 4> _vertexAttributes;
		VkPipelineVertexInputStateCreateInfo _vertexInput;
		VkPipelineInputAssemblyStateCreateInfo _inputAssembly;
		VkViewport _viewport;
		VkRect2D _scissor;
		VkPipelineViewportStateCreateInfo _viewportState;
		VkPipelineRasterizationStateCreateInfo _rasterizationState;
		VkPipelineMultisampleStateCreateInfo _multisampleState;
		VkPipelineDepthStencilStateCreateInfo _depthStencilState;
		VkPipelineColorBlendAttachmentState _colorBlendAttachmentState;
		VkPipelineColorBlendStateCreateInfo _colorBlendState;
		VkGraphicsPipelineCreateInfo _pipelineInfo;
	};
}
