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

		[[nodiscard]] constexpr VkDescriptorSetLayout descriptorSetLayout() const noexcept { return _descriptorSetLayout; }
		[[nodiscard]] constexpr VkPipelineLayout pipelineLayout() const noexcept { return _pipelineLayout; }
		[[nodiscard]] constexpr VkPipeline pipeline() const noexcept { return _pipeline; }
		void destroy() noexcept;

	private:
		friend VulkanPipelineBuilder;
		constexpr explicit VulkanPipeline(VkDevice device) noexcept
			: _device{ device } {}

	private:
		VkDevice _device = VK_NULL_HANDLE;
		VkDescriptorSetLayout _descriptorSetLayout = VK_NULL_HANDLE;
		VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
		VkPipeline _pipeline = VK_NULL_HANDLE;
	};

	class VulkanPipelineBuilder
	{
	public:
		explicit VulkanPipelineBuilder(const VkExtent2D&, VkSampleCountFlagBits, bool sampleShading) noexcept;

		VulkanPipeline build(VkDevice, VkRenderPass);
		void setDescriptorSetLayoutBinding(uint32_t binding, VkDescriptorType, VkShaderStageFlags) noexcept;
		void setInputAssembly(VkPrimitiveTopology, bool enablePrimitiveRestart) noexcept;
		void setStage(VkShaderStageFlagBits, VkShaderModule) noexcept;
		void setVertexInput(uint32_t binding, std::initializer_list<VertexAttribute>, VkVertexInputRate = VK_VERTEX_INPUT_RATE_VERTEX) noexcept;

	private:
		StaticVector<VkDescriptorSetLayoutBinding, 2> _descriptorSetLayoutBindings;
		VkDescriptorSetLayoutCreateInfo _descriptorSetLayoutInfo;
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
	, _descriptorSetLayout{ other._descriptorSetLayout }
	, _pipelineLayout{ other._pipelineLayout }
	, _pipeline{ other._pipeline }
{
	other._device = VK_NULL_HANDLE;
	other._descriptorSetLayout = VK_NULL_HANDLE;
	other._pipelineLayout = VK_NULL_HANDLE;
	other._pipeline = VK_NULL_HANDLE;
}
