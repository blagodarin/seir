// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "context.hpp"

#include <seir_app/window.hpp>
#include <seir_base/static_vector.hpp>
#include <seir_math/euler.hpp>
#include <seir_math/mat.hpp>

#include <algorithm>
#include <chrono>
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
		seir::Vec2 texCoord;
	};

	constexpr std::array kVertexData{
		Vertex{ .position{ -1, -1 }, .color{ 1, 0, 0 }, .texCoord{ 0, 0 } },
		Vertex{ .position{ 1, -1 }, .color{ 1, 1, 1 }, .texCoord{ 1, 0 } },
		Vertex{ .position{ -1, 1 }, .color{ 0, 1, 0 }, .texCoord{ 0, 1 } },
		Vertex{ .position{ 1, 1 }, .color{ 0, 0, 1 }, .texCoord{ 1, 1 } },
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
		VkVertexInputAttributeDescription{
			.location = 2,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(Vertex, color),
		},
	};

	struct UniformBufferObject
	{
		seir::Mat4 _model;
		seir::Mat4 _view;
		seir::Mat4 _projection;
	};

	VkImageView createImageView2D(VkDevice device, VkImage image, VkFormat format)
	{
		const VkImageViewCreateInfo createInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = image,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = format,
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
		VkImageView imageView = VK_NULL_HANDLE;
		SEIR_VK(vkCreateImageView(device, &createInfo, nullptr, &imageView));
		return imageView;
	}
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
		createDescriptorSetLayout(context._device);
		createPipelineLayout(context._device);
		createPipeline(context._device, context._vertexShader, context._fragmentShader);
		createFramebuffers(context._device);
		createUniformBuffers(context);
		createDescriptorPool(context._device);
		createDescriptorSets(context);
		createCommandBuffers(context._device, context._commandPool, context._vertexBuffer._buffer, context._indexBuffer._buffer);
		_swapchainImageFences.assign(_swapchainImages.size(), VK_NULL_HANDLE);
	}

	void VulkanSwapchain::destroy(VkDevice device, VkCommandPool commandPool) noexcept
	{
		std::fill(_swapchainImageFences.begin(), _swapchainImageFences.end(), VK_NULL_HANDLE);
		vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(_commandBuffers.size()), _commandBuffers.data());
		std::fill(_commandBuffers.begin(), _commandBuffers.end(), VK_NULL_HANDLE);
		std::fill(_descriptorSets.begin(), _descriptorSets.end(), VK_NULL_HANDLE);
		vkDestroyDescriptorPool(device, _descriptorPool, nullptr);
		for (auto& buffer : _uniformBuffers)
			buffer.destroy(device);
		for (auto& framebuffer : _swapchainFramebuffers)
		{
			vkDestroyFramebuffer(device, framebuffer, nullptr);
			framebuffer = VK_NULL_HANDLE;
		}
		vkDestroyPipeline(device, _pipeline, nullptr);
		_pipeline = VK_NULL_HANDLE;
		vkDestroyPipelineLayout(device, _pipelineLayout, nullptr);
		_pipelineLayout = VK_NULL_HANDLE;
		vkDestroyDescriptorSetLayout(device, _descriptorSetLayout, nullptr);
		_descriptorSetLayout = VK_NULL_HANDLE;
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

	void VulkanSwapchain::updateUniformBuffer(VkDevice device, uint32_t imageIndex)
	{
		static auto startTime = std::chrono::steady_clock::now();
		const float time = std::chrono::duration_cast<std::chrono::duration<float, std::chrono::seconds::period>>(std::chrono::steady_clock::now() - startTime).count();
		const UniformBufferObject ubo{
			._model = Mat4::rotation(30 * time, { 0, 0, 1 }),
			._view = Mat4::camera({ 0, -3, 3 }, { 0, -45, 0 }),
			._projection = Mat4::projection3D(static_cast<float>(_swapchainExtent.width) / static_cast<float>(_swapchainExtent.height), 45, 1),
		};
		_uniformBuffers[imageIndex].write(device, &ubo, sizeof ubo);
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
			_swapchainImageViews[i] = ::createImageView2D(device, _swapchainImages[i], surfaceFormat.format);
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

	void VulkanSwapchain::createDescriptorSetLayout(VkDevice device)
	{
		const std::array bindings{
			VkDescriptorSetLayoutBinding{
				.binding = 0,
				.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.descriptorCount = 1,
				.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
				.pImmutableSamplers = nullptr,
			},
			VkDescriptorSetLayoutBinding{
				.binding = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.descriptorCount = 1,
				.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
				.pImmutableSamplers = nullptr,
			},
		};
		const VkDescriptorSetLayoutCreateInfo createInfo{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = static_cast<uint32_t>(bindings.size()),
			.pBindings = bindings.data(),
		};
		SEIR_VK(vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &_descriptorSetLayout));
	}

	void VulkanSwapchain::createPipelineLayout(VkDevice device)
	{
		const VkPipelineLayoutCreateInfo createInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = 1,
			.pSetLayouts = &_descriptorSetLayout,
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
			.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
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

	void VulkanSwapchain::createUniformBuffers(const VulkanContext& context)
	{
		_uniformBuffers.resize(_swapchainImages.size());
		for (auto& buffer : _uniformBuffers)
			buffer.create(context, sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	}

	void VulkanSwapchain::createDescriptorPool(VkDevice device)
	{
		const std::array poolSizes{
			VkDescriptorPoolSize{
				.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.descriptorCount = static_cast<uint32_t>(_swapchainImages.size()),
			},
			VkDescriptorPoolSize{
				.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.descriptorCount = static_cast<uint32_t>(_swapchainImages.size()),
			},
		};
		const VkDescriptorPoolCreateInfo poolInfo{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.maxSets = static_cast<uint32_t>(_swapchainImages.size()),
			.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
			.pPoolSizes = poolSizes.data(),
		};
		SEIR_VK(vkCreateDescriptorPool(device, &poolInfo, nullptr, &_descriptorPool));
	}

	void VulkanSwapchain::createDescriptorSets(const VulkanContext& context)
	{
		const std::vector<VkDescriptorSetLayout> layouts(_swapchainImages.size(), _descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocateInfo{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = _descriptorPool,
			.descriptorSetCount = static_cast<uint32_t>(_swapchainImages.size()),
			.pSetLayouts = layouts.data(),
		};
		_descriptorSets.assign(_swapchainImages.size(), VK_NULL_HANDLE);
		SEIR_VK(vkAllocateDescriptorSets(context._device, &allocateInfo, _descriptorSets.data()));
		for (size_t i = 0; i < _swapchainImages.size(); ++i)
		{
			const VkDescriptorBufferInfo bufferInfo{
				.buffer = _uniformBuffers[i]._buffer,
				.offset = 0,
				.range = sizeof(UniformBufferObject),
			};
			const VkDescriptorImageInfo imageInfo{
				.sampler = context._textureSampler,
				.imageView = context._textureView,
				.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			};
			const std::array descriptorWrites{
				VkWriteDescriptorSet{
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.dstSet = _descriptorSets[i],
					.dstBinding = 0,
					.dstArrayElement = 0,
					.descriptorCount = 1,
					.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					.pImageInfo = nullptr,
					.pBufferInfo = &bufferInfo,
					.pTexelBufferView = nullptr,
				},
				VkWriteDescriptorSet{
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.dstSet = _descriptorSets[i],
					.dstBinding = 1,
					.dstArrayElement = 0,
					.descriptorCount = 1,
					.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.pImageInfo = &imageInfo,
					.pBufferInfo = nullptr,
					.pTexelBufferView = nullptr,
				},
			};
			vkUpdateDescriptorSets(context._device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
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
			vkCmdBindDescriptorSets(_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_descriptorSets[i], 0, nullptr);
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
		_buffer = VK_NULL_HANDLE;
		vkFreeMemory(device, _memory, nullptr);
		_memory = VK_NULL_HANDLE;
	}

	void VulkanBuffer::write(VkDevice device, const void* data, VkDeviceSize size, VkDeviceSize offset)
	{
		void* mapped = nullptr;
		SEIR_VK(vkMapMemory(device, _memory, offset, size, 0, &mapped));
		std::memcpy(mapped, data, size);
		vkUnmapMemory(device, _memory);
	}

	void VulkanImage::copy2D(const VulkanContext& context, VkBuffer buffer, uint32_t width, uint32_t height)
	{
		VulkanOneTimeSubmit commandBuffer{ context._device, context._commandPool };
		const VkBufferImageCopy region{
			.bufferOffset = 0,
			.bufferRowLength = 0,
			.bufferImageHeight = 0,
			.imageSubresource{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
			.imageOffset{
				.x = 0,
				.y = 0,
				.z = 0,
			},
			.imageExtent{
				.width = width,
				.height = height,
				.depth = 1,
			}
		};
		vkCmdCopyBufferToImage(commandBuffer, buffer, _image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		commandBuffer.submit(context._graphicsQueue);
	}

	void VulkanImage::createTexture2D(const VulkanContext& context, uint32_t width, uint32_t height, VkFormat format)
	{
		assert(_image == VK_NULL_HANDLE);
		const VkImageCreateInfo createInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = format,
			.extent{
				.width = width,
				.height = height,
				.depth = 1,
			},
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		};
		SEIR_VK(vkCreateImage(context._device, &createInfo, nullptr, &_image));
		VkMemoryRequirements memoryRequirements{};
		vkGetImageMemoryRequirements(context._device, _image, &memoryRequirements);
		const VkMemoryAllocateInfo allocateInfo{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = memoryRequirements.size,
			.memoryTypeIndex = context.findMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		};
		SEIR_VK(vkAllocateMemory(context._device, &allocateInfo, nullptr, &_memory));
		SEIR_VK(vkBindImageMemory(context._device, _image, _memory, 0));
	}

	void VulkanImage::destroy(VkDevice device) noexcept
	{
		vkDestroyImage(device, _image, nullptr);
		_image = VK_NULL_HANDLE;
		vkFreeMemory(device, _memory, nullptr);
		_memory = VK_NULL_HANDLE;
	}

	void VulkanImage::transitionLayout(const VulkanContext& context, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VulkanOneTimeSubmit commandBuffer{ context._device, context._commandPool };
		VkImageMemoryBarrier barrier{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.srcAccessMask = 0,
			.dstAccessMask = 0,
			.oldLayout = oldLayout,
			.newLayout = newLayout,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = _image,
			.subresourceRange{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			}
		};
		VkPipelineStageFlags srcStage = 0;
		VkPipelineStageFlags dstStage = 0;
		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else
			SEIR_VK_THROW("", "Unsupported layout transition");
		vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
		commandBuffer.submit(context._graphicsQueue);
	}

	VulkanOneTimeSubmit::VulkanOneTimeSubmit(VkDevice device, VkCommandPool commandPool)
		: _device{ device }, _commandPool{ commandPool }
	{
		const VkCommandBufferAllocateInfo allocateInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = _commandPool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1,
		};
		SEIR_VK(vkAllocateCommandBuffers(_device, &allocateInfo, &_commandBuffer));
		const VkCommandBufferBeginInfo beginInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		};
		SEIR_VK(vkBeginCommandBuffer(_commandBuffer, &beginInfo)); // TODO: Fix _commandBuffer leak.
	}

	VulkanOneTimeSubmit::~VulkanOneTimeSubmit() noexcept
	{
		vkFreeCommandBuffers(_device, _commandPool, 1, &_commandBuffer);
	}

	void VulkanOneTimeSubmit::submit(VkQueue queue)
	{
		SEIR_VK(vkEndCommandBuffer(_commandBuffer));
		VkSubmitInfo submitInfo{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.commandBufferCount = 1,
			.pCommandBuffers = &_commandBuffer,
		};
		SEIR_VK(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
		SEIR_VK(vkQueueWaitIdle(queue));
	}

	VulkanContext::~VulkanContext() noexcept
	{
		_indexBuffer.destroy(_device);
		_vertexBuffer.destroy(_device);
		vkDestroyShaderModule(_device, _fragmentShader, nullptr);
		vkDestroyShaderModule(_device, _vertexShader, nullptr);
		vkDestroySampler(_device, _textureSampler, nullptr);
		vkDestroyImageView(_device, _textureView, nullptr);
		_texture.destroy(_device);
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
		createTextureImage();
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
			vkGetPhysicalDeviceProperties(device, &_physicalDeviceProperties);

			VkPhysicalDeviceFeatures features{};
			vkGetPhysicalDeviceFeatures(device, &features);
			if (_useAnisotropy && !features.samplerAnisotropy)
				continue;

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
					std::cerr << "Vulkan physical device selected: " << _physicalDeviceProperties.deviceName << '\n';
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
		const VkPhysicalDeviceFeatures features{
			.samplerAnisotropy = _useAnisotropy ? VK_TRUE : VK_FALSE,
		};
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

	void VulkanContext::createTextureImage()
	{
		static constexpr std::array<uint8_t, 4> kImageData{ 0x99, 0xbb, 0xbb, 0xff };
		VulkanBuffer stagingBuffer; // TODO: Fix resource leak.
		stagingBuffer.create(*this, kImageData.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		stagingBuffer.write(_device, kImageData.data(), kImageData.size());
		_texture.createTexture2D(*this, 1, 1, VK_FORMAT_B8G8R8A8_SRGB);
		_texture.transitionLayout(*this, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		_texture.copy2D(*this, stagingBuffer._buffer, 1, 1);
		stagingBuffer.destroy(_device);
		_texture.transitionLayout(*this, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		_textureView = ::createImageView2D(_device, _texture._image, VK_FORMAT_B8G8R8A8_SRGB);
		const VkSamplerCreateInfo samplerInfo{
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.magFilter = VK_FILTER_NEAREST,
			.minFilter = VK_FILTER_NEAREST,
			.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
			.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			.mipLodBias = 0,
			.anisotropyEnable = _useAnisotropy ? VK_TRUE : VK_FALSE,
			.maxAnisotropy = _useAnisotropy ? _physicalDeviceProperties.limits.maxSamplerAnisotropy : 1,
			.compareEnable = VK_FALSE,
			.compareOp = VK_COMPARE_OP_ALWAYS,
			.minLod = 0,
			.maxLod = 0,
			.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
			.unnormalizedCoordinates = VK_FALSE,
		};
		SEIR_VK(vkCreateSampler(_device, &samplerInfo, nullptr, &_textureSampler));
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
		VulkanOneTimeSubmit commandBuffer{ _device, _commandPool };
		const VkBufferCopy region{
			.srcOffset = 0,
			.dstOffset = 0,
			.size = size,
		};
		vkCmdCopyBuffer(commandBuffer, src, dst, 1, &region);
		commandBuffer.submit(_graphicsQueue);
	}
}
