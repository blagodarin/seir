// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/static_vector.hpp>
#include "vulkan.hpp"

namespace seir
{
	class VulkanPipelineBuilder;

	enum class VertexAttribute
	{
		f32x2,
		f32x3,
	};

	class VulkanPipeline
	{
	public:
		constexpr VulkanPipeline() noexcept = default;
		constexpr VulkanPipeline(VulkanPipeline&&) noexcept;
		~VulkanPipeline() noexcept { destroy(); }
		VulkanPipeline& operator=(VulkanPipeline&&) noexcept;

		[[nodiscard]] constexpr VkDescriptorSetLayout descriptorSetLayout(size_t index) const noexcept { return _descriptorSetLayouts[index]; }
		[[nodiscard]] constexpr VkPipelineLayout pipelineLayout() const noexcept { return _pipelineLayout; }
		[[nodiscard]] constexpr VkPipeline pipeline() const noexcept { return _pipeline; }
		void destroy() noexcept;

	private:
		VkDevice _device = VK_NULL_HANDLE;
		StaticVector<VkDescriptorSetLayout, 2> _descriptorSetLayouts;
		VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
		VkPipeline _pipeline = VK_NULL_HANDLE;
		constexpr explicit VulkanPipeline(VkDevice device) noexcept
			: _device{ device } {}
		friend VulkanPipelineBuilder;
	};

	class VulkanPipelineBuilder
	{
	public:
		explicit VulkanPipelineBuilder(const VkExtent2D&, VkSampleCountFlagBits, bool sampleShading) noexcept;

		VulkanPipeline build(VkDevice, VkRenderPass);
		void addDescriptorSetLayout() noexcept;
		void setDescriptorSetLayoutBinding(uint32_t binding, VkDescriptorType, VkShaderStageFlags) noexcept;
		void setInputAssembly(VkPrimitiveTopology, bool enablePrimitiveRestart) noexcept;
		void setPushConstantRange(uint32_t offset, uint32_t size, VkShaderStageFlags) noexcept;
		void setStage(VkShaderStageFlagBits, VkShaderModule) noexcept;
		void setVertexInput(uint32_t binding, std::initializer_list<VertexAttribute>, VkVertexInputRate = VK_VERTEX_INPUT_RATE_VERTEX) noexcept;

	private:
		StaticVector<VkDescriptorSetLayoutBinding, 4> _descriptorSetLayoutBindings;
		StaticVector<VkDescriptorSetLayoutCreateInfo, 2> _descriptorSetLayouts;
		StaticVector<VkPushConstantRange, 1> _pushConstantRanges;
		VkPipelineLayoutCreateInfo _pipelineLayoutInfo;
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

constexpr seir::VulkanPipeline::VulkanPipeline(VulkanPipeline&& other) noexcept
	: _device{ other._device }
	, _pipelineLayout{ other._pipelineLayout }
	, _pipeline{ other._pipeline }
{
	for (const auto layout : other._descriptorSetLayouts)
		_descriptorSetLayouts.emplace_back(layout);
	other._device = VK_NULL_HANDLE;
	other._descriptorSetLayouts.clear();
	other._pipelineLayout = VK_NULL_HANDLE;
	other._pipeline = VK_NULL_HANDLE;
}
