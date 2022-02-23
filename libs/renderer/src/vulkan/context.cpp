// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "context.hpp"

#include <seir_app/window.hpp>
#include <seir_base/static_vector.hpp>
#include <seir_math/euler.hpp>
#include <seir_math/mat.hpp>
#include "error.hpp"
#include "pipeline.hpp"
#include "utils.hpp"

#include <unordered_set>

namespace
{
#ifndef NDEBUG
	void printInstanceInfo()
	{
		uint32_t count = 0;
		SEIR_VK(vkEnumerateInstanceLayerProperties(&count, nullptr));
		std::vector<VkLayerProperties> layers(count);
		SEIR_VK(vkEnumerateInstanceLayerProperties(&count, layers.data()));
		fmt::print(stderr, "Vulkan instance layers and extensions:\n");
		std::vector<VkExtensionProperties> extensions;
		SEIR_VK(vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr));
		extensions.resize(count);
		SEIR_VK(vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data()));
		for (const auto& extension : extensions)
			fmt::print(stderr, "   - {} - v.{}\n", extension.extensionName, extension.specVersion);
		for (const auto& layer : layers)
		{
			fmt::print(stderr, " * {} -- {}\n", layer.layerName, layer.description);
			SEIR_VK(vkEnumerateInstanceExtensionProperties(layer.layerName, &count, nullptr));
			extensions.resize(count);
			SEIR_VK(vkEnumerateInstanceExtensionProperties(layer.layerName, &count, extensions.data()));
			for (const auto& extension : extensions)
				fmt::print(stderr, "   - {} - v.{}\n", extension.extensionName, extension.specVersion);
		}
		fmt::print(stderr, "\n");
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* data, void*)
	{
		if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
			fmt::print(stderr, "{}\n", data->pMessage);
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

	const VkSurfaceFormatKHR* selectSurfaceFormat(VkPhysicalDevice device, VkSurfaceKHR surface, std::vector<VkSurfaceFormatKHR>& formats)
	{
		uint32_t count = 0;
		SEIR_VK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr));
		formats.resize(count);
		SEIR_VK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, formats.data()));
		if (formats.empty())
			return nullptr;
		for (const auto& format : formats)
			if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				return &format;
		return formats.data();
	}

	const VkPresentModeKHR* selectPresentMode(VkPhysicalDevice device, VkSurfaceKHR surface, std::vector<VkPresentModeKHR>& modes)
	{
		uint32_t count = 0;
		SEIR_VK(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, nullptr));
		modes.resize(count);
		SEIR_VK(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, modes.data()));
		const VkPresentModeKHR* fifoMode = nullptr;
		for (const auto& mode : modes)
		{
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
				return &mode;
			if (!fifoMode && mode == VK_PRESENT_MODE_FIFO_KHR)
				fifoMode = &mode;
		}
		return fifoMode;
	}

	const uint32_t kVertexShader[]{
#include "vertex_shader.glsl.spirv.inc"
	};

	const uint32_t kFragmentShader[]{
#include "fragment_shader.glsl.spirv.inc"
	};

	struct Vertex
	{
		seir::Vec3 position;
		seir::Vec3 color;
		seir::Vec2 texCoord;
	};

	constexpr std::array kVertexData{
		Vertex{ .position{ -1, -1, .5 }, .color{ 1, 0, 0 }, .texCoord{ 0, 0 } },
		Vertex{ .position{ 1, -1, .5 }, .color{ 1, 1, 1 }, .texCoord{ 1, 0 } },
		Vertex{ .position{ -1, 1, .5 }, .color{ 0, 1, 0 }, .texCoord{ 0, 1 } },
		Vertex{ .position{ 1, 1, .5 }, .color{ 0, 0, 1 }, .texCoord{ 1, 1 } },

		Vertex{ .position{ -1, -1, 0 }, .color{ 1, 1, 0 }, .texCoord{ 0, 0 } },
		Vertex{ .position{ 1, -1, 0 }, .color{ 0, 1, 1 }, .texCoord{ 1, 0 } },
		Vertex{ .position{ -1, 1, 0 }, .color{ 1, 0, 1 }, .texCoord{ 0, 1 } },
		Vertex{ .position{ 1, 1, 0 }, .color{ 0, 0, 0 }, .texCoord{ 1, 1 } },
	};

	constexpr std::array<uint16_t, 10> kIndexData{
		0, 1, 2, 3,
		0xffff,
		4, 5, 6, 7
	};

	struct UniformBufferObject
	{
		seir::Mat4 _model;
		seir::Mat4 _view;
		seir::Mat4 _projection;
	};

	VkImageView createImageView2D(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspect)
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
				.aspectMask = aspect,
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

	constexpr bool hasStencilComponent(VkFormat format) noexcept
	{
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
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

	void VulkanRenderTarget::create(const VulkanContext& context, const Size2D& windowSize)
	{
		createSwapchain(context, windowSize);
		createSwapchainImageViews(context._device, context._surfaceFormat);
		createColorBuffer(context);
		createDepthBuffer(context);
		createRenderPass(context._device, context._surfaceFormat.format, context._maxSampleCount);
		createFramebuffers(context._device);
		_swapchainImageFences.assign(_swapchainImages.size(), VK_NULL_HANDLE);
	}

	void VulkanRenderTarget::destroy(VkDevice device) noexcept
	{
		std::fill(_swapchainImageFences.begin(), _swapchainImageFences.end(), VK_NULL_HANDLE);
		for (auto& framebuffer : _swapchainFramebuffers)
		{
			vkDestroyFramebuffer(device, framebuffer, nullptr);
			framebuffer = VK_NULL_HANDLE;
		}
		vkDestroyRenderPass(device, _renderPass, nullptr);
		_renderPass = VK_NULL_HANDLE;
		vkDestroyImageView(device, _depthBufferView, nullptr);
		_depthBufferView = VK_NULL_HANDLE;
		_depthBuffer.destroy(device);
		vkDestroyImageView(device, _colorBufferView, nullptr);
		_colorBufferView = VK_NULL_HANDLE;
		_colorBuffer.destroy(device);
		for (auto& imageView : _swapchainImageViews)
		{
			vkDestroyImageView(device, imageView, nullptr);
			imageView = VK_NULL_HANDLE;
		}
		vkDestroySwapchainKHR(device, _swapchain, nullptr);
		_swapchain = VK_NULL_HANDLE;
	}

	void VulkanRenderTarget::createSwapchain(const VulkanContext& context, const Size2D& windowSize)
	{
		VkSurfaceCapabilitiesKHR surfaceCapabilities{};
		SEIR_VK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context._physicalDevice, context._surface, &surfaceCapabilities));

		_swapchainExtent = surfaceCapabilities.currentExtent;
		if (_swapchainExtent.width == std::numeric_limits<uint32_t>::max() || _swapchainExtent.height == std::numeric_limits<uint32_t>::max())
		{
			if (const auto width = static_cast<uint32_t>(windowSize._width); width < surfaceCapabilities.minImageExtent.width)
				_swapchainExtent.width = surfaceCapabilities.minImageExtent.width;
			else if (width > surfaceCapabilities.maxImageExtent.width)
				_swapchainExtent.width = surfaceCapabilities.maxImageExtent.width;
			else
				_swapchainExtent.width = width;
			if (const auto height = static_cast<uint32_t>(windowSize._width); height < surfaceCapabilities.minImageExtent.height)
				_swapchainExtent.height = surfaceCapabilities.minImageExtent.height;
			else if (height > surfaceCapabilities.maxImageExtent.height)
				_swapchainExtent.height = surfaceCapabilities.maxImageExtent.height;
			else
				_swapchainExtent.height = height;
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

	void VulkanRenderTarget::createSwapchainImageViews(VkDevice device, const VkSurfaceFormatKHR& surfaceFormat)
	{
		_swapchainImageViews.assign(_swapchainImages.size(), VK_NULL_HANDLE);
		for (size_t i = 0; i < _swapchainImages.size(); ++i)
			_swapchainImageViews[i] = ::createImageView2D(device, _swapchainImages[i], surfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT);
	}

	void VulkanRenderTarget::createColorBuffer(const VulkanContext& context)
	{
		if (context._maxSampleCount != VK_SAMPLE_COUNT_1_BIT)
		{
			_colorBuffer.createTexture2D(context, _swapchainExtent, context._surfaceFormat.format, context._maxSampleCount, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT);
			_colorBufferView = ::createImageView2D(context._device, _colorBuffer._image, context._surfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT);
		}
	}

	void VulkanRenderTarget::createDepthBuffer(const VulkanContext& context)
	{
		constexpr auto tiling = VK_IMAGE_TILING_OPTIMAL;
		_depthBufferFormat = context.findFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT }, tiling, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
		_depthBuffer.createTexture2D(context, _swapchainExtent, _depthBufferFormat, context._maxSampleCount, tiling, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
		_depthBuffer.transitionLayout(context, _depthBufferFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		_depthBufferView = ::createImageView2D(context._device, _depthBuffer._image, _depthBufferFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
	}

	void VulkanRenderTarget::createRenderPass(VkDevice device, VkFormat colorFormat, VkSampleCountFlagBits sampleCount)
	{
		StaticVector<VkAttachmentDescription, 3> attachments;
		attachments.emplace_back(VkAttachmentDescription{
			.format = colorFormat,
			.samples = sampleCount,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = sampleCount == VK_SAMPLE_COUNT_1_BIT
				? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
				: VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		});
		attachments.emplace_back(VkAttachmentDescription{
			.format = _depthBufferFormat,
			.samples = sampleCount,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		});
		if (sampleCount != VK_SAMPLE_COUNT_1_BIT)
			attachments.emplace_back(VkAttachmentDescription{
				.format = colorFormat,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			});

		const VkAttachmentReference colorReference{
			.attachment = 0,
			.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		};

		const VkAttachmentReference depthReference{
			.attachment = 1,
			.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		};

		const VkAttachmentReference resolveReference{
			.attachment = 2,
			.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		};

		const VkSubpassDescription subpass{
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.colorAttachmentCount = 1,
			.pColorAttachments = &colorReference,
			.pResolveAttachments = sampleCount != VK_SAMPLE_COUNT_1_BIT ? &resolveReference : nullptr,
			.pDepthStencilAttachment = &depthReference,
		};

		const VkSubpassDependency subpassDependency{
			.srcSubpass = VK_SUBPASS_EXTERNAL,
			.dstSubpass = 0,
			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			.srcAccessMask = 0,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			.dependencyFlags = 0,
		};

		const VkRenderPassCreateInfo createInfo{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.attachmentCount = static_cast<uint32_t>(attachments.size()),
			.pAttachments = attachments.data(),
			.subpassCount = 1,
			.pSubpasses = &subpass,
			.dependencyCount = 1,
			.pDependencies = &subpassDependency,
		};

		SEIR_VK(vkCreateRenderPass(device, &createInfo, nullptr, &_renderPass));
	}

	void VulkanRenderTarget::createFramebuffers(VkDevice device)
	{
		_swapchainFramebuffers.assign(_swapchainImageViews.size(), VK_NULL_HANDLE);
		for (size_t i = 0; i < _swapchainImageViews.size(); ++i)
		{
			StaticVector<VkImageView, 3> attachments;
			if (_colorBufferView)
			{
				attachments.emplace_back(_colorBufferView);
				attachments.emplace_back(_depthBufferView);
				attachments.emplace_back(_swapchainImageViews[i]);
			}
			else
			{
				attachments.emplace_back(_swapchainImageViews[i]);
				attachments.emplace_back(_depthBufferView);
			}
			const VkFramebufferCreateInfo createInfo{
				.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
				.renderPass = _renderPass,
				.attachmentCount = static_cast<uint32_t>(attachments.size()),
				.pAttachments = attachments.data(),
				.width = _swapchainExtent.width,
				.height = _swapchainExtent.height,
				.layers = 1,
			};
			SEIR_VK(vkCreateFramebuffer(device, &createInfo, nullptr, &_swapchainFramebuffers[i]));
		}
	}

	void VulkanSwapchain::create(const VulkanContext& context, const VulkanRenderTarget& renderTarget, const VulkanPipeline& pipeline)
	{
		const auto frameCount = static_cast<uint32_t>(renderTarget._swapchainImages.size());
		createUniformBuffers(context, frameCount);
		createDescriptorPool(context._device, frameCount);
		createDescriptorSets(context, pipeline.descriptorSetLayout(), frameCount);
		createCommandBuffers(context._device, context._commandPool, renderTarget, pipeline, context._vertexBuffer._buffer, context._indexBuffer._buffer);
	}

	void VulkanSwapchain::destroy(VkDevice device, VkCommandPool commandPool) noexcept
	{
		vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(_commandBuffers.size()), _commandBuffers.data());
		std::fill(_commandBuffers.begin(), _commandBuffers.end(), VK_NULL_HANDLE);
		std::fill(_descriptorSets.begin(), _descriptorSets.end(), VK_NULL_HANDLE);
		vkDestroyDescriptorPool(device, _descriptorPool, nullptr);
		for (auto& buffer : _uniformBuffers)
			buffer.destroy(device);
	}

	void VulkanSwapchain::updateUniformBuffer(VkDevice device, uint32_t imageIndex, const VkExtent2D& screenSize)
	{
		const auto time = clockTime();
		const UniformBufferObject ubo{
			._model = Mat4::rotation(10 * time, { 0, 0, 1 }),
			._view = Mat4::camera({ 0, -3, 3 }, { 0, -45, 0 }),
			._projection = Mat4::projection3D(static_cast<float>(screenSize.width) / static_cast<float>(screenSize.height), 45, 1),
		};
		_uniformBuffers[imageIndex].write(device, &ubo, sizeof ubo);
	}

	void VulkanSwapchain::createUniformBuffers(const VulkanContext& context, uint32_t count)
	{
		_uniformBuffers.resize(count);
		for (auto& buffer : _uniformBuffers)
			buffer.create(context, sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	}

	void VulkanSwapchain::createDescriptorPool(VkDevice device, uint32_t count)
	{
		const std::array poolSizes{
			VkDescriptorPoolSize{
				.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.descriptorCount = count,
			},
			VkDescriptorPoolSize{
				.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.descriptorCount = count,
			},
		};
		const VkDescriptorPoolCreateInfo poolInfo{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.maxSets = count,
			.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
			.pPoolSizes = poolSizes.data(),
		};
		SEIR_VK(vkCreateDescriptorPool(device, &poolInfo, nullptr, &_descriptorPool));
	}

	void VulkanSwapchain::createDescriptorSets(const VulkanContext& context, VkDescriptorSetLayout layout, uint32_t count)
	{
		const std::vector<VkDescriptorSetLayout> layouts(count, layout);
		VkDescriptorSetAllocateInfo allocateInfo{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = _descriptorPool,
			.descriptorSetCount = count,
			.pSetLayouts = layouts.data(),
		};
		_descriptorSets.assign(count, VK_NULL_HANDLE);
		SEIR_VK(vkAllocateDescriptorSets(context._device, &allocateInfo, _descriptorSets.data()));
		for (uint32_t i = 0; i < count; ++i)
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

	void VulkanSwapchain::createCommandBuffers(VkDevice device, VkCommandPool commandPool, const VulkanRenderTarget& renderTarget, const VulkanPipeline& pipeline, VkBuffer vertexBuffer, VkBuffer indexBuffer)
	{
		_commandBuffers.assign(renderTarget._swapchainFramebuffers.size(), VK_NULL_HANDLE);
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
			const std::array clearValues{
				VkClearValue{
					.color{
						.float32{ 0.f, 0.f, 0.f, 1.f },
					},
				},
				VkClearValue{
					.depthStencil{
						.depth = 0,
						.stencil = 0,
					},
				}
			};
			const VkRenderPassBeginInfo renderPassInfo{
				.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
				.renderPass = renderTarget._renderPass,
				.framebuffer = renderTarget._swapchainFramebuffers[i],
				.renderArea{
					.offset{ 0, 0 },
					.extent = renderTarget._swapchainExtent,
				},
				.clearValueCount = static_cast<uint32_t>(clearValues.size()),
				.pClearValues = clearValues.data(),
			};
			vkCmdBeginRenderPass(_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline());
			VkBuffer vertexBuffers[]{ vertexBuffer };
			VkDeviceSize offsets[]{ 0 };
			vkCmdBindVertexBuffers(_commandBuffers[i], 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(_commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT16);
			vkCmdBindDescriptorSets(_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipelineLayout(), 0, 1, &_descriptorSets[i], 0, nullptr);
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

	void VulkanImage::createTexture2D(const VulkanContext& context, const VkExtent2D& extent, VkFormat format, VkSampleCountFlagBits sampleCount, VkImageTiling tiling, VkImageUsageFlags usage)
	{
		assert(_image == VK_NULL_HANDLE);
		const VkImageCreateInfo createInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = format,
			.extent{
				.width = extent.width,
				.height = extent.height,
				.depth = 1,
			},
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = sampleCount,
			.tiling = tiling,
			.usage = usage,
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

	void VulkanImage::transitionLayout(const VulkanContext& context, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
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
				.aspectMask = newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
					? (hasStencilComponent(format)
							? static_cast<VkImageAspectFlags>(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)
							: VK_IMAGE_ASPECT_DEPTH_BIT)
					: VK_IMAGE_ASPECT_COLOR_BIT,
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
		else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
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
			if ((_options.anisotropicFiltering && !features.samplerAnisotropy)
				|| (_options.sampleShading && !features.sampleRateShading)) // TODO: Use the best device even if it doesn't have all supported features.
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
			auto graphicsQueue = queueFamilyCount;
			auto presentQueue = queueFamilyCount;
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
					_physicalDevice = device;
					_surfaceFormat = *surfaceFormat;
					_presentMode = *presentMode;
					_graphicsQueueFamily = graphicsQueue;
					_presentQueueFamily = presentQueue;
					if (_options.multisampleAntialiasing)
					{
						if (const auto sampleCountMask = _physicalDeviceProperties.limits.framebufferColorSampleCounts & _physicalDeviceProperties.limits.framebufferDepthSampleCounts;
							sampleCountMask & VK_SAMPLE_COUNT_64_BIT)
							_maxSampleCount = VK_SAMPLE_COUNT_64_BIT;
						else if (sampleCountMask & VK_SAMPLE_COUNT_32_BIT)
							_maxSampleCount = VK_SAMPLE_COUNT_32_BIT;
						else if (sampleCountMask & VK_SAMPLE_COUNT_16_BIT)
							_maxSampleCount = VK_SAMPLE_COUNT_16_BIT;
						else if (sampleCountMask & VK_SAMPLE_COUNT_8_BIT)
							_maxSampleCount = VK_SAMPLE_COUNT_8_BIT;
						else if (sampleCountMask & VK_SAMPLE_COUNT_4_BIT)
							_maxSampleCount = VK_SAMPLE_COUNT_4_BIT;
						else if (sampleCountMask & VK_SAMPLE_COUNT_2_BIT)
							_maxSampleCount = VK_SAMPLE_COUNT_2_BIT;
					}
#ifndef NDEBUG
					fmt::print(stderr, "Vulkan physical device selected: {}\n", _physicalDeviceProperties.deviceName);
					fmt::print(stderr, "Vulkan device extensions:\n");
					for (const auto& extension : extensions)
						fmt::print(stderr, "   - {} - v.{}\n", extension.extensionName, extension.specVersion);
					fmt::print(stderr, "Vulkan MSAA sample count: {}\n", _maxSampleCount);
					fmt::print(stderr, "\n");
#endif
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
			.sampleRateShading = static_cast<VkBool32>(_options.sampleShading),
			.samplerAnisotropy = static_cast<VkBool32>(_options.anisotropicFiltering),
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
		constexpr auto format = VK_FORMAT_B8G8R8A8_SRGB;
		_texture.createTexture2D(*this, { 1, 1 }, format, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		_texture.transitionLayout(*this, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		_texture.copy2D(*this, stagingBuffer._buffer, 1, 1);
		stagingBuffer.destroy(_device);
		_texture.transitionLayout(*this, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		_textureView = ::createImageView2D(_device, _texture._image, format, VK_IMAGE_ASPECT_COLOR_BIT);
		const VkSamplerCreateInfo samplerInfo{
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.magFilter = VK_FILTER_NEAREST,
			.minFilter = VK_FILTER_NEAREST,
			.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
			.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			.mipLodBias = 0,
			.anisotropyEnable = static_cast<VkBool32>(_options.anisotropicFiltering),
			.maxAnisotropy = _options.anisotropicFiltering ? _physicalDeviceProperties.limits.maxSamplerAnisotropy : 1,
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

	VkFormat VulkanContext::findFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const
	{
		VkFormatProperties properties{};
		for (const auto format : candidates)
		{
			vkGetPhysicalDeviceFormatProperties(_physicalDevice, format, &properties);
			if (tiling == VK_IMAGE_TILING_OPTIMAL)
			{
				if ((properties.optimalTilingFeatures & features) == features)
					return format;
			}
			else if (tiling == VK_IMAGE_TILING_LINEAR)
			{
				if ((properties.linearTilingFeatures & features) == features)
					return format;
			}
		}
		SEIR_VK_THROW("vkGetPhysicalDeviceFormatProperties", "No suitable supported format found");
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
