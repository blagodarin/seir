// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "context.hpp"

#include <seir_app/window.hpp>
#include <seir_base/static_vector.hpp>
#include <seir_math/vec.hpp>

#include <algorithm>
#include <optional>
#include <unordered_set>

#ifndef NDEBUG
#	include <iostream>
#endif

namespace
{
#ifndef NDEBUG
	void printInstanceInfo()
	{
		uint32_t count = 0;
		SEIR_VK(vkEnumerateInstanceLayerProperties(&count, nullptr));
		std::vector<VkLayerProperties> layers(count);
		SEIR_VK(vkEnumerateInstanceLayerProperties(&count, layers.data()));
		std::cerr << "Vulkan instance layers and extensions:\n";
		std::vector<VkExtensionProperties> extensions;
		SEIR_VK(vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr));
		extensions.resize(count);
		SEIR_VK(vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data()));
		for (const auto& extension : extensions)
			std::cerr << "   - " << extension.extensionName << " v." << extension.specVersion << "\n";
		for (const auto& layer : layers)
		{
			std::cerr << " * " << layer.layerName << " -- " << layer.description << "\n";
			SEIR_VK(vkEnumerateInstanceExtensionProperties(layer.layerName, &count, nullptr));
			extensions.resize(count);
			SEIR_VK(vkEnumerateInstanceExtensionProperties(layer.layerName, &count, extensions.data()));
			for (const auto& extension : extensions)
				std::cerr << "   - " << extension.extensionName << " - v" << extension.specVersion << "\n";
		}
		std::cerr << '\n';
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* data, void*)
	{
		if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
			std::cerr << data->pMessage << '\n';
		return VK_FALSE;
	}

	void fillDebugUtilsMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& info) noexcept
	{
		info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		info.pfnUserCallback = ::debugUtilsMessengerCallback;
	}
#endif

	constexpr std::array kRequiredDeviceExtensions{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};

	bool checkDeviceExtensions(VkPhysicalDevice device, std::vector<VkExtensionProperties>& extensions)
	{
		uint32_t extensionCount = 0;
		SEIR_VK(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr));
		extensions.resize(extensionCount);
		SEIR_VK(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data()));
		std::unordered_set<std::string_view> extensionMap;
		extensionMap.reserve(extensions.size());
		for (const auto& extension : extensions)
			extensionMap.emplace(extension.extensionName);
		for (const auto requiredExtension : kRequiredDeviceExtensions)
			if (!extensionMap.contains(requiredExtension))
				return false;
		return true;
	}

	std::optional<VkSurfaceFormatKHR> selectSurfaceFormat(VkPhysicalDevice device, VkSurfaceKHR surface, std::vector<VkSurfaceFormatKHR>& formats)
	{
		uint32_t count = 0;
		SEIR_VK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr));
		formats.resize(count);
		SEIR_VK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, formats.data()));
		if (formats.empty())
			return {};
		for (const auto& format : formats)
			if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				return format;
		return formats.front();
	}

	std::optional<VkPresentModeKHR> selectPresentMode(VkPhysicalDevice device, VkSurfaceKHR surface, std::vector<VkPresentModeKHR>& modes)
	{
		uint32_t count = 0;
		SEIR_VK(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, nullptr));
		modes.resize(count);
		SEIR_VK(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, modes.data()));
		bool hasFifoMode = false;
		for (const auto mode : modes)
		{
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
				return mode;
			if (!hasFifoMode && mode == VK_PRESENT_MODE_FIFO_KHR)
				hasFifoMode = true;
		}
		if (hasFifoMode)
			return VK_PRESENT_MODE_FIFO_KHR;
		return {};
	}

	const uint32_t kVertexShader[]{
#include "vertex_shader.glsl.spirv.inc"
	};

	const uint32_t kFragmentShader[]{
#include "fragment_shader.glsl.spirv.inc"
	};

	struct Vertex
	{
		seir::Vec2 position;
		seir::Vec3 color;
	};

	constexpr std::array kVertexData{
		Vertex{ .position{ -.75f, -.75f }, .color{ 1.f, 0.f, 0.f } },
		Vertex{ .position{ .75f, -.75f }, .color{ 0.f, 1.f, 0.f } },
		Vertex{ .position{ -.75f, .75f }, .color{ 0.f, 1.f, 0.f } },
		Vertex{ .position{ .75f, .75f }, .color{ 0.f, 0.f, 1.f } },
	};

	constexpr std::array<uint16_t, 4> kIndexData{ 0, 1, 2, 3 };

	constexpr VkVertexInputBindingDescription kVertexBinding{
		.binding = 0,
		.stride = sizeof(Vertex),
		.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
	};

	constexpr std::array kVertexAttributes{
		VkVertexInputAttributeDescription{
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(Vertex, position),
		},
		VkVertexInputAttributeDescription{
			.location = 1,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(Vertex, color),
		},
	};
}

namespace seir
{
	void VulkanFrameSync::create(VkDevice device)
	{
		const VkSemaphoreCreateInfo semaphoreInfo{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		};
		const VkFenceCreateInfo fenceInfo{
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT,
		};
		for (auto& item : _items)
		{
			SEIR_VK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &item._imageAvailableSemaphore));
			SEIR_VK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &item._renderFinishedSemaphore));
			SEIR_VK(vkCreateFence(device, &fenceInfo, nullptr, &item._fence));
		}
	}

	void VulkanFrameSync::destroy(VkDevice device) noexcept
	{
		for (auto& item : _items)
		{
			vkDestroySemaphore(device, item._imageAvailableSemaphore, nullptr);
			item._imageAvailableSemaphore = VK_NULL_HANDLE;
			vkDestroySemaphore(device, item._renderFinishedSemaphore, nullptr);
			item._renderFinishedSemaphore = VK_NULL_HANDLE;
			vkDestroyFence(device, item._fence, nullptr);
			item._fence = VK_NULL_HANDLE;
		}
	}

	VulkanFrameSync::Item VulkanFrameSync::switchFrame(VkDevice device)
	{
		SEIR_VK(vkWaitForFences(device, 1, &_items[_index]._fence, VK_TRUE, UINT64_MAX));
		const auto index = _index;
		_index = (index + 1) % _items.size();
		return _items[index];
	}

	void VulkanSwapchain::create(const VulkanContext& context, const Size2D& windowSize)
	{
		createSwapchain(context, windowSize);
		createSwapchainImageViews(context._device, context._surfaceFormat);
		createRenderPass(context._device, context._surfaceFormat);
		createPipelineLayout(context._device);
		createPipeline(context._device, context._vertexShader, context._fragmentShader);
		createFramebuffers(context._device);
		createCommandBuffers(context._device, context._commandPool, context._vertexBuffer._buffer, context._indexBuffer._buffer);
		_swapchainImageFences.assign(_swapchainImages.size(), VK_NULL_HANDLE);
	}

	void VulkanSwapchain::destroy(VkDevice device, VkCommandPool commandPool) noexcept
	{
		vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(_commandBuffers.size()), _commandBuffers.data());
		std::fill(_commandBuffers.begin(), _commandBuffers.end(), VK_NULL_HANDLE);
		for (auto& framebuffer : _swapchainFramebuffers)
		{
			vkDestroyFramebuffer(device, framebuffer, nullptr);
			framebuffer = VK_NULL_HANDLE;
		}
		vkDestroyPipeline(device, _pipeline, nullptr);
		_pipeline = VK_NULL_HANDLE;
		vkDestroyPipelineLayout(device, _pipelineLayout, nullptr);
		_pipelineLayout = VK_NULL_HANDLE;
		vkDestroyRenderPass(device, _renderPass, nullptr);
		_renderPass = VK_NULL_HANDLE;
		for (auto& imageView : _swapchainImageViews)
		{
			vkDestroyImageView(device, imageView, nullptr);
			imageView = VK_NULL_HANDLE;
		}
		vkDestroySwapchainKHR(device, _swapchain, nullptr);
		_swapchain = VK_NULL_HANDLE;
	}

	void VulkanSwapchain::createSwapchain(const VulkanContext& context, const Size2D& windowSize)
	{
		VkSurfaceCapabilitiesKHR surfaceCapabilities{};
		SEIR_VK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context._physicalDevice, context._surface, &surfaceCapabilities));

		_swapchainExtent = surfaceCapabilities.currentExtent;
		if (_swapchainExtent.width == std::numeric_limits<uint32_t>::max() || _swapchainExtent.height == std::numeric_limits<uint32_t>::max())
		{
			_swapchainExtent.width = std::clamp(static_cast<uint32_t>(windowSize._width), surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
			_swapchainExtent.height = std::clamp(static_cast<uint32_t>(windowSize._height), surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
		}

		auto imageCount = surfaceCapabilities.minImageCount + 1;
		if (surfaceCapabilities.maxImageCount > 0 && imageCount > surfaceCapabilities.maxImageCount)
			imageCount = surfaceCapabilities.maxImageCount;

		const std::array queueFamilies{ context._graphicsQueueFamily, context._presentQueueFamily };
		VkSwapchainCreateInfoKHR createInfo{
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.surface = context._surface,
			.minImageCount = imageCount,
			.imageFormat = context._surfaceFormat.format,
			.imageColorSpace = context._surfaceFormat.colorSpace,
			.imageExtent = _swapchainExtent,
			.imageArrayLayers = 1,
			.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			.preTransform = surfaceCapabilities.currentTransform,
			.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			.presentMode = context._presentMode,
			.clipped = VK_TRUE,
		};
		if (context._graphicsQueueFamily != context._presentQueueFamily)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilies.size());
			createInfo.pQueueFamilyIndices = queueFamilies.data();
		}
		else
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		SEIR_VK(vkCreateSwapchainKHR(context._device, &createInfo, nullptr, &_swapchain));
		SEIR_VK(vkGetSwapchainImagesKHR(context._device, _swapchain, &imageCount, nullptr));
		_swapchainImages.resize(imageCount);
		SEIR_VK(vkGetSwapchainImagesKHR(context._device, _swapchain, &imageCount, _swapchainImages.data()));
	}

	void VulkanSwapchain::createSwapchainImageViews(VkDevice device, const VkSurfaceFormatKHR& surfaceFormat)
	{
		_swapchainImageViews.assign(_swapchainImages.size(), VK_NULL_HANDLE);
		for (size_t i = 0; i < _swapchainImages.size(); ++i)
		{
			const VkImageViewCreateInfo createInfo{
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.image = _swapchainImages[i],
				.viewType = VK_IMAGE_VIEW_TYPE_2D,
				.format = surfaceFormat.format,
				.components{
					.r = VK_COMPONENT_SWIZZLE_IDENTITY,
					.g = VK_COMPONENT_SWIZZLE_IDENTITY,
					.b = VK_COMPONENT_SWIZZLE_IDENTITY,
					.a = VK_COMPONENT_SWIZZLE_IDENTITY,
				},
				.subresourceRange{
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1,
				},
			};
			SEIR_VK(vkCreateImageView(device, &createInfo, nullptr, &_swapchainImageViews[i]));
		}
	}

	void VulkanSwapchain::createRenderPass(VkDevice device, const VkSurfaceFormatKHR& surfaceFormat)
	{
		const VkAttachmentDescription colorAttachment{
			.format = surfaceFormat.format,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		};

		const VkAttachmentReference colorReference{
			.attachment = 0,
			.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		};

		const VkSubpassDescription subpass{
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.colorAttachmentCount = 1,
			.pColorAttachments = &colorReference,
		};

		const VkSubpassDependency subpassDependency{
			.srcSubpass = VK_SUBPASS_EXTERNAL,
			.dstSubpass = 0,
			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.srcAccessMask = 0,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			.dependencyFlags = 0,
		};

		const VkRenderPassCreateInfo createInfo{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.attachmentCount = 1,
			.pAttachments = &colorAttachment,
			.subpassCount = 1,
			.pSubpasses = &subpass,
			.dependencyCount = 1,
			.pDependencies = &subpassDependency,
		};

		SEIR_VK(vkCreateRenderPass(device, &createInfo, nullptr, &_renderPass));
	}

	void VulkanSwapchain::createPipelineLayout(VkDevice device)
	{
		const VkPipelineLayoutCreateInfo createInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = 0,
			.pSetLayouts = nullptr,
			.pushConstantRangeCount = 0,
			.pPushConstantRanges = nullptr,
		};

		SEIR_VK(vkCreatePipelineLayout(device, &createInfo, nullptr, &_pipelineLayout));
	}

	void VulkanSwapchain::createPipeline(VkDevice device, VkShaderModule vertexShader, VkShaderModule fragmentShader)
	{
		const std::array shaderStages{
			VkPipelineShaderStageCreateInfo{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.stage = VK_SHADER_STAGE_VERTEX_BIT,
				.module = vertexShader,
				.pName = "main",
			},
			VkPipelineShaderStageCreateInfo{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
				.module = fragmentShader,
				.pName = "main",
			},
		};

		const VkPipelineVertexInputStateCreateInfo vertexInputState{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			.vertexBindingDescriptionCount = 1,
			.pVertexBindingDescriptions = &kVertexBinding,
			.vertexAttributeDescriptionCount = static_cast<uint32_t>(kVertexAttributes.size()),
			.pVertexAttributeDescriptions = kVertexAttributes.data(),
		};

		const VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
			.primitiveRestartEnable = VK_FALSE,
		};

		const VkViewport viewport{
			.x = 0.f,
			.y = 0.f,
			.width = static_cast<float>(_swapchainExtent.width),
			.height = static_cast<float>(_swapchainExtent.height),
			.minDepth = 0.f,
			.maxDepth = 1.f,
		};

		const VkRect2D scissor{
			.offset = { 0, 0 },
			.extent = _swapchainExtent,
		};

		const VkPipelineViewportStateCreateInfo viewportState{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			.viewportCount = 1,
			.pViewports = &viewport,
			.scissorCount = 1,
			.pScissors = &scissor,
		};

		const VkPipelineRasterizationStateCreateInfo rasterizationState{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.depthClampEnable = VK_FALSE,
			.rasterizerDiscardEnable = VK_FALSE,
			.polygonMode = VK_POLYGON_MODE_FILL,
			.cullMode = VK_CULL_MODE_BACK_BIT,
			.frontFace = VK_FRONT_FACE_CLOCKWISE,
			.depthBiasEnable = VK_FALSE,
			.depthBiasConstantFactor = 0.f,
			.depthBiasClamp = 0.f,
			.depthBiasSlopeFactor = 0.f,
			.lineWidth = 1.f,
		};

		const VkPipelineMultisampleStateCreateInfo multisampleState{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
			.sampleShadingEnable = VK_FALSE,
			.minSampleShading = 1.f,
			.pSampleMask = nullptr,
			.alphaToCoverageEnable = VK_FALSE,
			.alphaToOneEnable = VK_FALSE,
		};

		const VkPipelineColorBlendAttachmentState colorBlendAttachment{
			.blendEnable = VK_FALSE,
			.srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
			.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
			.colorBlendOp = VK_BLEND_OP_ADD,
			.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
			.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
			.alphaBlendOp = VK_BLEND_OP_ADD,
			.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		};

		const VkPipelineColorBlendStateCreateInfo colorBlendState{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.logicOpEnable = VK_FALSE,
			.logicOp = VK_LOGIC_OP_COPY,
			.attachmentCount = 1,
			.pAttachments = &colorBlendAttachment,
			.blendConstants{ 0.f, 0.f, 0.f, 0.f },
		};

		const VkGraphicsPipelineCreateInfo pipelineInfo{
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.stageCount = static_cast<uint32_t>(shaderStages.size()),
			.pStages = shaderStages.data(),
			.pVertexInputState = &vertexInputState,
			.pInputAssemblyState = &inputAssemblyState,
			.pTessellationState = nullptr,
			.pViewportState = &viewportState,
			.pRasterizationState = &rasterizationState,
			.pMultisampleState = &multisampleState,
			.pDepthStencilState = nullptr,
			.pColorBlendState = &colorBlendState,
			.pDynamicState = nullptr,
			.layout = _pipelineLayout,
			.renderPass = _renderPass,
			.subpass = 0,
			.basePipelineHandle = VK_NULL_HANDLE,
			.basePipelineIndex = 0,
		};

		SEIR_VK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_pipeline));
	}

	void VulkanSwapchain::createFramebuffers(VkDevice device)
	{
		_swapchainFramebuffers.assign(_swapchainImageViews.size(), VK_NULL_HANDLE);
		for (size_t i = 0; i < _swapchainImageViews.size(); ++i)
		{
			const VkFramebufferCreateInfo createInfo{
				.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
				.renderPass = _renderPass,
				.attachmentCount = 1,
				.pAttachments = &_swapchainImageViews[i],
				.width = _swapchainExtent.width,
				.height = _swapchainExtent.height,
				.layers = 1,
			};
			SEIR_VK(vkCreateFramebuffer(device, &createInfo, nullptr, &_swapchainFramebuffers[i]));
		}
	}

	void VulkanSwapchain::createCommandBuffers(VkDevice device, VkCommandPool commandPool, VkBuffer vertexBuffer, VkBuffer indexBuffer)
	{
		_commandBuffers.assign(_swapchainFramebuffers.size(), VK_NULL_HANDLE);
		const VkCommandBufferAllocateInfo allocateInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = commandPool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = static_cast<uint32_t>(_commandBuffers.size()),
		};
		SEIR_VK(vkAllocateCommandBuffers(device, &allocateInfo, _commandBuffers.data()));
		for (size_t i = 0; i < _commandBuffers.size(); ++i)
		{
			const VkCommandBufferBeginInfo info{
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				.flags = 0,
				.pInheritanceInfo = nullptr,
			};
			SEIR_VK(vkBeginCommandBuffer(_commandBuffers[i], &info));
			const VkClearValue clearColor{
				.color{
					.float32{ 0.f, 0.f, 0.f, 1.f },
				},
			};
			const VkRenderPassBeginInfo renderPassInfo{
				.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
				.renderPass = _renderPass,
				.framebuffer = _swapchainFramebuffers[i],
				.renderArea{
					.offset{ 0, 0 },
					.extent = _swapchainExtent,
				},
				.clearValueCount = 1,
				.pClearValues = &clearColor,
			};
			vkCmdBeginRenderPass(_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
			VkBuffer vertexBuffers[]{ vertexBuffer };
			VkDeviceSize offsets[]{ 0 };
			vkCmdBindVertexBuffers(_commandBuffers[i], 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(_commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT16);
			vkCmdDrawIndexed(_commandBuffers[i], static_cast<uint32_t>(kIndexData.size()), 1, 0, 0, 0);
			vkCmdEndRenderPass(_commandBuffers[i]);
			SEIR_VK(vkEndCommandBuffer(_commandBuffers[i]));
		}
	}

	void VulkanBuffer::create(const VulkanContext& context, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
	{
		assert(_buffer == VK_NULL_HANDLE);
		const VkBufferCreateInfo createInfo{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = size,
			.usage = usage,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		};
		SEIR_VK(vkCreateBuffer(context._device, &createInfo, nullptr, &_buffer));
		VkMemoryRequirements memoryRequirements{};
		vkGetBufferMemoryRequirements(context._device, _buffer, &memoryRequirements);
		const VkMemoryAllocateInfo allocateInfo{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = memoryRequirements.size,
			.memoryTypeIndex = context.findMemoryType(memoryRequirements.memoryTypeBits, properties)
		};
		SEIR_VK(vkAllocateMemory(context._device, &allocateInfo, nullptr, &_memory));
		SEIR_VK(vkBindBufferMemory(context._device, _buffer, _memory, 0));
	}

	void VulkanBuffer::destroy(VkDevice device) noexcept
	{
		vkDestroyBuffer(device, _buffer, nullptr);
		vkFreeMemory(device, _memory, nullptr);
	}

	void VulkanBuffer::write(VkDevice device, const void* data, VkDeviceSize size, VkDeviceSize offset)
	{
		void* mapped = nullptr;
		SEIR_VK(vkMapMemory(device, _memory, offset, size, 0, &mapped));
		std::memcpy(mapped, data, size);
		vkUnmapMemory(device, _memory);
	}

	VulkanContext::~VulkanContext() noexcept
	{
		_indexBuffer.destroy(_device);
		_vertexBuffer.destroy(_device);
		vkDestroyShaderModule(_device, _fragmentShader, nullptr);
		vkDestroyShaderModule(_device, _vertexShader, nullptr);
		vkDestroyCommandPool(_device, _commandPool, nullptr);
		vkDestroyDevice(_device, nullptr);
		vkDestroySurfaceKHR(_instance, _surface, nullptr);
#ifndef NDEBUG
		if (_vkDestroyDebugUtilsMessenger)
			_vkDestroyDebugUtilsMessenger(_instance, _debugUtilsMessenger, nullptr);
#endif
		vkDestroyInstance(_instance, nullptr);
	}

	void VulkanContext::create(const WindowDescriptor& windowDescriptor)
	{
#ifndef NDEBUG
		::printInstanceInfo();
#endif
		createInstance();
#ifndef NDEBUG
		createDebugUtilsMessenger();
#endif
		createSurface(windowDescriptor);
		selectPhysicalDevice();
		createDevice();
		createCommandPool();
		_vertexShader = loadShader(kVertexShader, sizeof kVertexShader);
		_fragmentShader = loadShader(kFragmentShader, sizeof kFragmentShader);
		createVertexBuffer();
		createIndexBuffer();
	}

	void VulkanContext::createInstance()
	{
		const VkApplicationInfo applicationInfo{
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.apiVersion = VK_API_VERSION_1_0,
		};
		static const std::array layers{
#ifndef NDEBUG
			"VK_LAYER_KHRONOS_validation",
#endif
			"", // Extra value for std::array type deduction.
		};
		static const std::array extensions{
#ifndef NDEBUG
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
			VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_WIN32_KHR
			VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
		};
		VkInstanceCreateInfo createInfo{
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pApplicationInfo = &applicationInfo,
			.enabledLayerCount = static_cast<uint32_t>(layers.size() - 1),
			.ppEnabledLayerNames = layers.data(),
			.enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
			.ppEnabledExtensionNames = extensions.data(),
		};
#ifndef NDEBUG
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		::fillDebugUtilsMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = &debugCreateInfo;
#endif
		SEIR_VK(vkCreateInstance(&createInfo, nullptr, &_instance));
	}

#ifndef NDEBUG
	void VulkanContext::createDebugUtilsMessenger()
	{
		const auto vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(_instance, "vkCreateDebugUtilsMessengerEXT"));
		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		::fillDebugUtilsMessengerCreateInfo(createInfo);
		SEIR_VK(vkCreateDebugUtilsMessengerEXT(_instance, &createInfo, nullptr, &_debugUtilsMessenger));
		_vkDestroyDebugUtilsMessenger = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(_instance, "vkDestroyDebugUtilsMessengerEXT"));
	}
#endif

	void VulkanContext::createSurface(const WindowDescriptor& windowDescriptor)
	{
#ifdef VK_USE_PLATFORM_WIN32_KHR
		const VkWin32SurfaceCreateInfoKHR createInfo{
			.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
			.hinstance = static_cast<HINSTANCE>(windowDescriptor._app),
			.hwnd = reinterpret_cast<HWND>(windowDescriptor._window),
		};
		SEIR_VK(vkCreateWin32SurfaceKHR(_instance, &createInfo, nullptr, &_surface));
#endif
	}

	void VulkanContext::selectPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		SEIR_VK(vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr));
		std::vector<VkPhysicalDevice> devices(deviceCount);
		SEIR_VK(vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.data()));
		std::vector<VkExtensionProperties> extensions;
		std::vector<VkSurfaceFormatKHR> surfaceFormats;
		std::vector<VkPresentModeKHR> presentModes;
		std::vector<VkQueueFamilyProperties> queueFamilies;
		for (const auto device : devices)
		{
			VkPhysicalDeviceProperties properties{}; // TODO: Use.
			vkGetPhysicalDeviceProperties(device, &properties);

			VkPhysicalDeviceFeatures features{}; // TODO: Use.
			vkGetPhysicalDeviceFeatures(device, &features);

			if (!::checkDeviceExtensions(device, extensions))
				continue;

			const auto surfaceFormat = ::selectSurfaceFormat(device, _surface, surfaceFormats);
			if (!surfaceFormat)
				continue;

			const auto presentMode = ::selectPresentMode(device, _surface, presentModes);
			if (!presentMode)
				continue;

			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
			queueFamilies.resize(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
			uint32_t graphicsQueue = queueFamilyCount;
			uint32_t presentQueue = queueFamilyCount;
			for (uint32_t i = 0; i < queueFamilyCount; ++i)
			{
				if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
					graphicsQueue = i;
				VkBool32 supported = VK_FALSE;
				SEIR_VK(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _surface, &supported));
				if (supported)
					presentQueue = i;
				if (graphicsQueue < queueFamilyCount && presentQueue < queueFamilyCount)
				{
#ifndef NDEBUG
					std::cerr << "Vulkan physical device selected: " << properties.deviceName << '\n';
					std::cerr << "Vulkan device extensions:\n";
					for (const auto& extension : extensions)
						std::cerr << "   - " << extension.extensionName << " v." << extension.specVersion << "\n";
					std::cerr << '\n';
#endif
					_physicalDevice = device;
					_surfaceFormat = *surfaceFormat;
					_presentMode = *presentMode;
					_graphicsQueueFamily = graphicsQueue;
					_presentQueueFamily = presentQueue;
					return;
				}
			}
		}
		SEIR_VK_THROW("", "No supported physical device found");
	}

	void VulkanContext::createDevice()
	{
		StaticVector<VkDeviceQueueCreateInfo, 2> queues;
		const float queuePriority = 1;
		queues.push_back({ .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = _graphicsQueueFamily,
			.queueCount = 1,
			.pQueuePriorities = &queuePriority });
		if (_presentQueueFamily != _graphicsQueueFamily)
			queues.push_back({ .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = _presentQueueFamily,
				.queueCount = 1,
				.pQueuePriorities = &queuePriority });
		// Device layers are deprecated, but it is still recommended to specify them.
		static const std::array layers{
#ifndef NDEBUG
			"VK_LAYER_KHRONOS_validation",
#endif
			"", // Extra value for std::array type deduction.
		};
		const VkPhysicalDeviceFeatures features{};
		const VkDeviceCreateInfo createInfo{
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.queueCreateInfoCount = static_cast<uint32_t>(queues.size()),
			.pQueueCreateInfos = queues.data(),
			.enabledLayerCount = static_cast<uint32_t>(layers.size() - 1),
			.ppEnabledLayerNames = layers.data(),
			.enabledExtensionCount = static_cast<uint32_t>(kRequiredDeviceExtensions.size()),
			.ppEnabledExtensionNames = kRequiredDeviceExtensions.data(),
			.pEnabledFeatures = &features,
		};
		SEIR_VK(vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device));
		vkGetDeviceQueue(_device, _graphicsQueueFamily, 0, &_graphicsQueue);
		vkGetDeviceQueue(_device, _presentQueueFamily, 0, &_presentQueue);
	}

	void VulkanContext::createCommandPool()
	{
		const VkCommandPoolCreateInfo createInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.queueFamilyIndex = _graphicsQueueFamily,
		};
		SEIR_VK(vkCreateCommandPool(_device, &createInfo, nullptr, &_commandPool));
	}

	VkShaderModule VulkanContext::loadShader(const uint32_t* data, size_t size)
	{
		const VkShaderModuleCreateInfo createInfo{
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.codeSize = size,
			.pCode = data,
		};
		VkShaderModule shaderModule = VK_NULL_HANDLE;
		SEIR_VK(vkCreateShaderModule(_device, &createInfo, nullptr, &shaderModule));
		return shaderModule;
	}

	uint32_t VulkanContext::findMemoryType(uint32_t filter, VkMemoryPropertyFlags properties) const
	{
		VkPhysicalDeviceMemoryProperties memoryProperties{};
		vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memoryProperties);
		for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
			if (filter & (1 << i) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
				return i;
		SEIR_VK_THROW("vkGetPhysicalDeviceMemoryProperties", "No suitable memory type found");
	}

	void VulkanContext::createVertexBuffer()
	{
		const auto size = kVertexData.size() * sizeof(Vertex);
		VulkanBuffer stagingBuffer; // TODO: Fix resource leak.
		stagingBuffer.create(*this, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		stagingBuffer.write(_device, kVertexData.data(), size);
		_vertexBuffer.create(*this, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		copyBuffer(_vertexBuffer._buffer, stagingBuffer._buffer, size);
		stagingBuffer.destroy(_device);
	}

	void VulkanContext::createIndexBuffer()
	{
		const auto size = kIndexData.size() * sizeof(uint16_t);
		VulkanBuffer stagingBuffer; // TODO: Fix resource leak.
		stagingBuffer.create(*this, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		stagingBuffer.write(_device, kIndexData.data(), size);
		_indexBuffer.create(*this, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		copyBuffer(_indexBuffer._buffer, stagingBuffer._buffer, size);
		stagingBuffer.destroy(_device);
	}

	void VulkanContext::copyBuffer(VkBuffer dst, VkBuffer src, VkDeviceSize size)
	{
		const VkCommandBufferAllocateInfo allocateInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = _commandPool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1,
		};
		VkCommandBuffer commandBuffer = VK_NULL_HANDLE; // TODO: Fix resource leak.
		SEIR_VK(vkAllocateCommandBuffers(_device, &allocateInfo, &commandBuffer));
		const VkCommandBufferBeginInfo beginInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		};
		SEIR_VK(vkBeginCommandBuffer(commandBuffer, &beginInfo));
		const VkBufferCopy region{
			.srcOffset = 0,
			.dstOffset = 0,
			.size = size,
		};
		vkCmdCopyBuffer(commandBuffer, src, dst, 1, &region);
		SEIR_VK(vkEndCommandBuffer(commandBuffer));
		VkSubmitInfo submitInfo{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.commandBufferCount = 1,
			.pCommandBuffers = &commandBuffer,
		};
		SEIR_VK(vkQueueSubmit(_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
		SEIR_VK(vkQueueWaitIdle(_graphicsQueue));
		vkFreeCommandBuffers(_device, _commandPool, 1, &commandBuffer);
	}
}