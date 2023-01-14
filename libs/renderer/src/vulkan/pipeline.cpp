// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "pipeline.hpp"

#include <seir_renderer/mesh.hpp>
#include "error.hpp"

namespace seir
{
	VulkanPipeline& VulkanPipeline::operator=(VulkanPipeline&& other) noexcept
	{
		destroy();
		_device = other._device;
		for (const auto layout : other._descriptorSetLayouts)
			_descriptorSetLayouts.emplace_back(layout);
		_pipelineLayout = other._pipelineLayout;
		_pipeline = other._pipeline;
		other._device = VK_NULL_HANDLE;
		other._descriptorSetLayouts.clear();
		other._pipelineLayout = VK_NULL_HANDLE;
		other._pipeline = VK_NULL_HANDLE;
		return *this;
	}

	void VulkanPipeline::destroy() noexcept
	{
		if (_pipeline)
		{
			vkDestroyPipeline(_device, _pipeline, nullptr);
			_pipeline = VK_NULL_HANDLE;
		}
		if (_pipelineLayout)
		{
			vkDestroyPipelineLayout(_device, _pipelineLayout, nullptr);
			_pipelineLayout = VK_NULL_HANDLE;
		}
		for (const auto layout : _descriptorSetLayouts)
			if (layout)
				vkDestroyDescriptorSetLayout(_device, layout, nullptr);
		_descriptorSetLayouts.clear();
		_device = VK_NULL_HANDLE;
	}

	VulkanPipelineBuilder::VulkanPipelineBuilder(const VkExtent2D& extent, VkSampleCountFlagBits sampleCount, bool sampleShading) noexcept
		: _pipelineLayoutInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = 0,
			.pSetLayouts = VK_NULL_HANDLE,
			.pushConstantRangeCount = 0,
			.pPushConstantRanges = _pushConstantRanges.data(),
		}
		, _vertexInput{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			.vertexBindingDescriptionCount = 0,
			.pVertexBindingDescriptions = _vertexInputBindings.data(),
			.vertexAttributeDescriptionCount = 0,
			.pVertexAttributeDescriptions = _vertexAttributes.data(),
		}
		, _inputAssembly{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
			.primitiveRestartEnable = VK_FALSE,
		}
		, _viewport{
			.x = 0.f,
			.y = 0.f,
			.width = static_cast<float>(extent.width),
			.height = static_cast<float>(extent.height),
			.minDepth = 0.f,
			.maxDepth = 1.f,
		}
		, _scissor{
			.offset = { 0, 0 },
			.extent = extent,
		}
		, _viewportState{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			.viewportCount = 1,
			.pViewports = &_viewport,
			.scissorCount = 1,
			.pScissors = &_scissor,
		}
		, _rasterizationState{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.depthClampEnable = VK_FALSE,
			.rasterizerDiscardEnable = VK_FALSE,
			.polygonMode = VK_POLYGON_MODE_FILL,
			.cullMode = VK_CULL_MODE_BACK_BIT,
			.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
			.depthBiasEnable = VK_FALSE,
			.depthBiasConstantFactor = 0.f,
			.depthBiasClamp = 0.f,
			.depthBiasSlopeFactor = 0.f,
			.lineWidth = 1.f,
		}
		, _multisampleState{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.rasterizationSamples = sampleCount,
			.sampleShadingEnable = static_cast<VkBool32>(sampleShading),
			.minSampleShading = sampleShading ? .25f : 1.f,
			.pSampleMask = nullptr,
			.alphaToCoverageEnable = VK_FALSE,
			.alphaToOneEnable = VK_FALSE,
		}
		, _depthStencilState{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.depthTestEnable = VK_TRUE,
			.depthWriteEnable = VK_TRUE,
			.depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL,
			.depthBoundsTestEnable = VK_FALSE,
			.stencilTestEnable = VK_FALSE,
			.front{},
			.back{},
			.minDepthBounds = 0,
			.maxDepthBounds = 1,
		}
		, _colorBlendAttachmentState{
			.blendEnable = VK_FALSE,
			.srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
			.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
			.colorBlendOp = VK_BLEND_OP_ADD,
			.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
			.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
			.alphaBlendOp = VK_BLEND_OP_ADD,
			.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		}
		, _colorBlendState{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.logicOpEnable = VK_FALSE,
			.logicOp = VK_LOGIC_OP_COPY,
			.attachmentCount = 1,
			.pAttachments = &_colorBlendAttachmentState,
			.blendConstants{ 0.f, 0.f, 0.f, 0.f },
		}
		, _pipelineInfo{
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.stageCount = 0,
			.pStages = _stages.data(),
			.pVertexInputState = &_vertexInput,
			.pInputAssemblyState = &_inputAssembly,
			.pTessellationState = nullptr,
			.pViewportState = &_viewportState,
			.pRasterizationState = &_rasterizationState,
			.pMultisampleState = &_multisampleState,
			.pDepthStencilState = &_depthStencilState,
			.pColorBlendState = &_colorBlendState,
		}
	{
	}

	VulkanPipeline VulkanPipelineBuilder::build(VkDevice device, VkRenderPass renderPass)
	{
		VulkanPipeline pipeline{ device };
		for (auto bindings = _descriptorSetLayoutBindings.data(); auto& layout : _descriptorSetLayouts)
		{
			layout.pBindings = bindings;
			bindings += layout.bindingCount;
			pipeline._descriptorSetLayouts.emplace_back(VK_NULL_HANDLE);
			SEIR_VK(vkCreateDescriptorSetLayout(device, &layout, nullptr, &pipeline._descriptorSetLayouts.back()));
		}
		_pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(_descriptorSetLayouts.size());
		_pipelineLayoutInfo.pSetLayouts = pipeline._descriptorSetLayouts.data();
		_pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(_pushConstantRanges.size());
		SEIR_VK(vkCreatePipelineLayout(device, &_pipelineLayoutInfo, nullptr, &pipeline._pipelineLayout));
		_vertexInput.vertexBindingDescriptionCount = static_cast<uint32_t>(_vertexInputBindings.size());
		_vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(_vertexAttributes.size());
		_pipelineInfo.stageCount = static_cast<uint32_t>(_stages.size());
		_pipelineInfo.layout = pipeline._pipelineLayout;
		_pipelineInfo.renderPass = renderPass;
		SEIR_VK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &_pipelineInfo, nullptr, &pipeline._pipeline));
		return pipeline;
	}

	void VulkanPipelineBuilder::addDescriptorSetLayout() noexcept
	{
		_descriptorSetLayouts.emplace_back(VkDescriptorSetLayoutCreateInfo{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = 0,
		});
	}

	void VulkanPipelineBuilder::setDescriptorSetLayoutBinding(uint32_t binding, VkDescriptorType type, VkShaderStageFlags flags) noexcept
	{
		assert(!_descriptorSetLayouts.empty());
		_descriptorSetLayoutBindings.emplace_back(VkDescriptorSetLayoutBinding{
			.binding = binding,
			.descriptorType = type,
			.descriptorCount = 1,
			.stageFlags = flags,
			.pImmutableSamplers = nullptr,
		});
		++_descriptorSetLayouts.back().bindingCount;
	}

	void VulkanPipelineBuilder::setInputAssembly(MeshTopology topology) noexcept
	{
		switch (topology)
		{
		case MeshTopology::TriangleList:
			_inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			_inputAssembly.primitiveRestartEnable = VK_FALSE;
			break;
		case MeshTopology::TriangleStrip:
			_inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
			_inputAssembly.primitiveRestartEnable = VK_TRUE;
			break;
		}
	}

	void VulkanPipelineBuilder::setPushConstantRange(uint32_t offset, uint32_t size, VkShaderStageFlags flags) noexcept
	{
		_pushConstantRanges.emplace_back(VkPushConstantRange{
			.stageFlags = flags,
			.offset = offset,
			.size = size,
		});
	}

	void VulkanPipelineBuilder::setStage(VkShaderStageFlagBits stage, VkShaderModule shader) noexcept
	{
		_stages.emplace_back(VkPipelineShaderStageCreateInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = stage,
			.module = shader,
			.pName = "main",
		});
	}

	void VulkanPipelineBuilder::setVertexInput(uint32_t binding, std::span<const VertexAttribute> attributes, VkVertexInputRate rate) noexcept
	{
		uint32_t location = 0;
		uint32_t offset = 0;
		for (const auto attribute : attributes)
		{
			VkFormat format = VK_FORMAT_UNDEFINED;
			uint32_t size = 0;
			switch (attribute)
			{
			case VertexAttribute::f32x2:
				format = VK_FORMAT_R32G32_SFLOAT;
				size = 2 * sizeof(float);
				break;
			case VertexAttribute::f32x3:
				format = VK_FORMAT_R32G32B32_SFLOAT;
				size = 3 * sizeof(float);
				break;
			}
			_vertexAttributes.emplace_back(VkVertexInputAttributeDescription{
				.location = location++,
				.binding = binding,
				.format = format,
				.offset = offset,
			});
			offset += size;
		}
		_vertexInputBindings.emplace_back(VkVertexInputBindingDescription{
			.binding = binding,
			.stride = offset,
			.inputRate = rate,
		});
	}
}
