// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "context.hpp"

#include <seir_app/window.hpp>
#include <seir_base/int_utils.hpp>
#include <seir_graphics/size.hpp>
#include "commands.hpp"
#include "error.hpp"
#include "pipeline.hpp"
#include "utils.hpp"

#include <unordered_set>

#define DEBUG_RENDERER 0 // TODO: Redesign debug info collection.

#if !defined(NDEBUG) && DEBUG_RENDERER
#	include <fmt/core.h>
#endif

namespace
{
#if !defined(NDEBUG) && DEBUG_RENDERER
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
#endif

#ifndef NDEBUG
	VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* data, void*)
	{
		if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
			std::fprintf(stderr, "%s\n", data->pMessage);
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

	void VulkanFrameSync::resize(VkDevice device, uint32_t requiredSize)
	{
		if (requiredSize <= _items.size())
			return;
		_items.resize(requiredSize);
		const VkSemaphoreCreateInfo semaphoreInfo{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		};
		const VkFenceCreateInfo fenceInfo{
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT,
		};
		for (auto& item : _items)
		{
			if (!item._imageAvailableSemaphore)
				SEIR_VK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &item._imageAvailableSemaphore));
			if (!item._renderFinishedSemaphore)
				SEIR_VK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &item._renderFinishedSemaphore));
			if (!item._fence)
				SEIR_VK(vkCreateFence(device, &fenceInfo, nullptr, &item._fence));
		}
	}

	VulkanFrameSync::Item VulkanFrameSync::switchFrame(VkDevice device)
	{
		SEIR_VK(vkWaitForFences(device, 1, &_items[_index]._fence, VK_TRUE, UINT64_MAX));
		const auto index = _index;
		_index = (index + 1) % _items.size();
		return _items[index];
	}

	bool VulkanRenderTarget::acquireFrame(VkDevice device, VkSemaphore signalSemaphore, VkFence waitFence, uint32_t& index)
	{
		if (const auto status = vkAcquireNextImageKHR(device, _swapchain, UINT64_MAX, signalSemaphore, VK_NULL_HANDLE, &index);
			status != VK_SUCCESS && status != VK_SUBOPTIMAL_KHR)
		{
			if (status != VK_ERROR_OUT_OF_DATE_KHR)
				SEIR_VK_THROW("vkAcquireNextImageKHR", status);
			return false;
		}
		if (_swapchainImageFences[index])
			SEIR_VK(vkWaitForFences(device, 1, &_swapchainImageFences[index], VK_TRUE, UINT64_MAX));
		_swapchainImageFences[index] = waitFence;
		return true;
	}

	void VulkanRenderTarget::create(const VulkanContext& context, const Size& windowSize)
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
		_depthBuffer.destroy();
		_colorBuffer.destroy();
		for (auto& imageView : _swapchainImageViews)
		{
			vkDestroyImageView(device, imageView, nullptr);
			imageView = VK_NULL_HANDLE;
		}
		vkDestroySwapchainKHR(device, _swapchain, nullptr);
		_swapchain = VK_NULL_HANDLE;
	}

	bool VulkanRenderTarget::presentFrame(VkQueue queue, uint32_t frameIndex, VkSemaphore waitSemaphore)
	{
		const VkPresentInfoKHR presentInfo{
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &waitSemaphore,
			.swapchainCount = 1,
			.pSwapchains = &_swapchain,
			.pImageIndices = &frameIndex,
			.pResults = nullptr,
		};
		if (const auto status = vkQueuePresentKHR(queue, &presentInfo); status != VK_SUCCESS)
		{
			if (status != VK_ERROR_OUT_OF_DATE_KHR && status != VK_SUBOPTIMAL_KHR)
				SEIR_VK_THROW("vkQueuePresentKHR", status);
			return false;
		}
		return true;
	}

	VkRenderPassBeginInfo VulkanRenderTarget::renderPassInfo(size_t frameIndex) const noexcept
	{
		static const std::array clearValues{
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
		return VkRenderPassBeginInfo{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.renderPass = _renderPass,
			.framebuffer = _swapchainFramebuffers[frameIndex],
			.renderArea{
				.offset{ 0, 0 },
				.extent = _swapchainExtent,
			},
			.clearValueCount = static_cast<uint32_t>(clearValues.size()),
			.pClearValues = clearValues.data(),
		};
	}

	void VulkanRenderTarget::createSwapchain(const VulkanContext& context, const Size& windowSize)
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
			_colorBuffer = context.createImage2D(_swapchainExtent, context._surfaceFormat.format, context._maxSampleCount, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	}

	void VulkanRenderTarget::createDepthBuffer(const VulkanContext& context)
	{
		constexpr auto tiling = VK_IMAGE_TILING_OPTIMAL;
		const auto format = context.findFormat({ VK_FORMAT_X8_D24_UNORM_PACK32, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT }, tiling, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
		_depthBuffer = context.createImage2D(_swapchainExtent, format, context._maxSampleCount, tiling, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
		_depthBuffer.transitionLayout(context, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
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
			.format = _depthBuffer.format(),
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
			if (_colorBuffer.handle())
			{
				attachments.emplace_back(_colorBuffer.viewHandle());
				attachments.emplace_back(_depthBuffer.viewHandle());
				attachments.emplace_back(_swapchainImageViews[i]);
			}
			else
			{
				attachments.emplace_back(_swapchainImageViews[i]);
				attachments.emplace_back(_depthBuffer.viewHandle());
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

	VulkanUniformBuffers& VulkanUniformBuffers::operator=(VulkanUniformBuffers&& other) noexcept
	{
		_bufferSize = other._bufferSize;
		_buffers = std::move(other._buffers);
		return *this;
	}

	void VulkanUniformBuffers::destroy() noexcept
	{
		for (auto& buffer : _buffers)
			buffer.destroy();
	}

	void VulkanUniformBuffers::update(size_t index, const void* data)
	{
		_buffers[index].write(data, _bufferSize);
	}

	VulkanBuffer& VulkanBuffer::operator=(VulkanBuffer&& other) noexcept
	{
		destroy();
		_allocator = other._allocator;
		_buffer = other._buffer;
		_allocation = other._allocation;
		other._allocator = VK_NULL_HANDLE;
		other._buffer = VK_NULL_HANDLE;
		other._allocation = VK_NULL_HANDLE;
		return *this;
	}

	void VulkanBuffer::destroy() noexcept
	{
		if (!_allocator)
			return;
		vmaDestroyBuffer(_allocator, _buffer, _allocation);
		_allocator = VK_NULL_HANDLE;
		_buffer = VK_NULL_HANDLE;
		_allocation = VK_NULL_HANDLE;
	}

	void* VulkanBuffer::map()
	{
		void* mapped = nullptr;
		SEIR_VK(vmaMapMemory(_allocator, _allocation, &mapped));
		return mapped;
	}

	void VulkanBuffer::write(const void* data, VkDeviceSize size)
	{
		void* mapped = nullptr;
		SEIR_VK(vmaMapMemory(_allocator, _allocation, &mapped));
		std::memcpy(mapped, data, size);
		vmaUnmapMemory(_allocator, _allocation);
	}

	VulkanImage& VulkanImage::operator=(VulkanImage&& other) noexcept
	{
		destroy();
		_allocator = other._allocator;
		_image = other._image;
		_allocation = other._allocation;
		_device = other._device;
		_view = other._view;
		_format = other._format;
		other._allocator = VK_NULL_HANDLE;
		other._image = VK_NULL_HANDLE;
		other._allocation = VK_NULL_HANDLE;
		other._device = VK_NULL_HANDLE;
		other._view = VK_NULL_HANDLE;
		other._format = VK_FORMAT_UNDEFINED;
		return *this;
	}

	void VulkanImage::copy2D(const VulkanContext& context, VkBuffer buffer, const VkExtent2D& extent, uint32_t pixelStride)
	{
		auto commandBuffer = context.createCommandBuffer(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		const VkBufferImageCopy region{
			.bufferOffset = 0,
			.bufferRowLength = pixelStride,
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
				.width = extent.width,
				.height = extent.height,
				.depth = 1,
			}
		};
		vkCmdCopyBufferToImage(commandBuffer, buffer, _image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		commandBuffer.finishAndSubmit(context._graphicsQueue);
	}

	void VulkanImage::destroy() noexcept
	{
		if (_device)
		{
			vkDestroyImageView(_device, _view, nullptr);
			_device = VK_NULL_HANDLE;
			_view = VK_NULL_HANDLE;
		}
		if (_allocator)
		{
			vmaDestroyImage(_allocator, _image, _allocation);
			_allocator = VK_NULL_HANDLE;
			_image = VK_NULL_HANDLE;
			_allocation = VK_NULL_HANDLE;
		}
		_format = VK_FORMAT_UNDEFINED;
	}

	void VulkanImage::transitionLayout(const VulkanContext& context, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
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
					? (hasStencilComponent(_format)
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
		auto commandBuffer = context.createCommandBuffer(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
		commandBuffer.finishAndSubmit(context._graphicsQueue);
	}

	VulkanSampler& VulkanSampler::operator=(VulkanSampler&& other) noexcept
	{
		destroy();
		_device = other._device;
		_sampler = other._sampler;
		other._device = VK_NULL_HANDLE;
		other._sampler = VK_NULL_HANDLE;
		return *this;
	}

	void VulkanSampler::destroy() noexcept
	{
		if (_sampler)
		{
			vkDestroySampler(_device, _sampler, nullptr);
			_device = VK_NULL_HANDLE;
			_sampler = VK_NULL_HANDLE;
		}
	}

	VulkanShader& VulkanShader::operator=(VulkanShader&& other) noexcept
	{
		destroy();
		_device = other._device;
		_module = other._module;
		other._device = VK_NULL_HANDLE;
		other._module = VK_NULL_HANDLE;
		return *this;
	}

	void VulkanShader::destroy() noexcept
	{
		if (_module)
		{
			vkDestroyShaderModule(_device, _module, nullptr);
			_device = VK_NULL_HANDLE;
			_module = VK_NULL_HANDLE;
		}
	}

	VulkanContext::~VulkanContext() noexcept
	{
		vkDestroyCommandPool(_device, _commandPool, nullptr);
		vmaDestroyAllocator(_allocator);
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
#if !defined(NDEBUG) && DEBUG_RENDERER
		::printInstanceInfo();
#endif
		createInstance();
#ifndef NDEBUG
		createDebugUtilsMessenger();
#endif
		createSurface(windowDescriptor);
		selectPhysicalDevice();
		createDevice();
		createAllocator();
		createCommandPool();
	}

	VulkanBuffer VulkanContext::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VmaAllocationCreateFlags allocationFlags) const
	{
		assert(_allocator);
		VulkanBuffer buffer{ _allocator };
		const VkBufferCreateInfo createInfo{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = size,
			.usage = usage,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		};
		const VmaAllocationCreateInfo allocateInfo{
			.flags = allocationFlags,
			.usage = VMA_MEMORY_USAGE_AUTO,
			.requiredFlags = properties,
			.preferredFlags = 0,
			.memoryTypeBits = 0,
			.pool = VK_NULL_HANDLE,
			.pUserData = nullptr,
			.priority = 0,
		};
		SEIR_VK(vmaCreateBuffer(_allocator, &createInfo, &allocateInfo, &buffer._buffer, &buffer._allocation, nullptr));
		return buffer;
	}

	vulkan::CommandBuffer VulkanContext::createCommandBuffer(VkCommandBufferUsageFlags usage) const
	{
		vulkan::CommandBuffer commandBuffer{ _device, _commandPool };
		const VkCommandBufferAllocateInfo allocateInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = _commandPool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1,
		};
		SEIR_VK(vkAllocateCommandBuffers(_device, &allocateInfo, &commandBuffer._buffer));
		const VkCommandBufferBeginInfo beginInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = usage,
		};
		SEIR_VK(vkBeginCommandBuffer(commandBuffer, &beginInfo));
		return commandBuffer;
	}

	VulkanBuffer VulkanContext::createDeviceBuffer(const void* data, VkDeviceSize size, VkBufferUsageFlags usage) const
	{
		auto stagingBuffer = createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
		stagingBuffer.write(data, size);
		auto buffer = createBuffer(size, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0);
		copyBuffer(buffer.handle(), stagingBuffer.handle(), size);
		return buffer;
	}

	VulkanImage VulkanContext::createImage2D(const VkExtent2D& extent, VkFormat format, VkSampleCountFlagBits sampleCount, VkImageTiling tiling, VkImageUsageFlags usage, VkImageAspectFlags aspect) const
	{
		assert(_allocator);
		VulkanImage image{ _allocator, _device, format };
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
		const VmaAllocationCreateInfo allocateInfo{
			.flags = 0,
			.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
			.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			.preferredFlags = 0,
			.memoryTypeBits = 0,
			.pool = VK_NULL_HANDLE,
			.pUserData = nullptr,
			.priority = 0,
		};
		SEIR_VK(vmaCreateImage(_allocator, &createInfo, &allocateInfo, &image._image, &image._allocation, nullptr));
		image._view = ::createImageView2D(_device, image._image, format, aspect);
		return image;
	}

	VulkanSampler VulkanContext::createSampler2D() const
	{
		assert(_device);
		VulkanSampler sampler{ _device };
		const VkSamplerCreateInfo createInfo{
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
		SEIR_VK(vkCreateSampler(_device, &createInfo, nullptr, &sampler._sampler));
		return sampler;
	}

	VulkanShader VulkanContext::createShader(const uint32_t* data, size_t bytes) const
	{
		assert(_device);
		VulkanShader shader{ _device };
		const VkShaderModuleCreateInfo createInfo{
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.codeSize = bytes,
			.pCode = data,
		};
		SEIR_VK(vkCreateShaderModule(_device, &createInfo, nullptr, &shader._module));
		return shader;
	}

	VulkanImage VulkanContext::createTextureImage2D(const VkExtent2D& extent, VkFormat format, VkDeviceSize size, const void* data, uint32_t pixelStride) const
	{
		auto image = createImage2D(extent, format, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
		image.transitionLayout(*this, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		{
			auto stagingBuffer = createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
			stagingBuffer.write(data, size);
			image.copy2D(*this, stagingBuffer.handle(), extent, pixelStride);
		}
		image.transitionLayout(*this, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		return image;
	}

	VulkanUniformBuffers VulkanContext::createUniformBuffers(VkDeviceSize size, size_t count) const
	{
		// TODO: Allocate one VkBuffer for all instances.
		VulkanUniformBuffers buffers{ size };
		buffers._buffers.resize(count);
		for (auto& buffer : buffers._buffers)
			buffer = createBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
		return buffers;
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
#if !defined(NDEBUG) && DEBUG_RENDERER
					fmt::print(stderr, "Vulkan device extensions:\n");
					for (const auto& extension : extensions)
						fmt::print(stderr, "   - {} - v.{}\n", extension.extensionName, extension.specVersion);
					fmt::print(stderr, "Vulkan MSAA sample count: {}\n", toUnderlying(_maxSampleCount));
					fmt::print(stderr, "\n");
					fmt::print(stderr, "[VkPhysicalDeviceProperties]\n");
					fmt::print(stderr, "deviceName = {}\n", _physicalDeviceProperties.deviceName);
					fmt::print(stderr, "\n");
					fmt::print(stderr, "[VkPhysicalDeviceLimits]\n");
					fmt::print(stderr, "maxImageDimension1D = {}\n", _physicalDeviceProperties.limits.maxImageDimension1D);
					fmt::print(stderr, "maxImageDimension2D = {}\n", _physicalDeviceProperties.limits.maxImageDimension2D);
					fmt::print(stderr, "maxImageDimension3D = {}\n", _physicalDeviceProperties.limits.maxImageDimension3D);
					fmt::print(stderr, "maxImageDimensionCube = {}\n", _physicalDeviceProperties.limits.maxImageDimensionCube);
					fmt::print(stderr, "maxImageArrayLayers = {}\n", _physicalDeviceProperties.limits.maxImageArrayLayers);
					fmt::print(stderr, "maxTexelBufferElements = {}\n", _physicalDeviceProperties.limits.maxTexelBufferElements);
					fmt::print(stderr, "maxUniformBufferRange = {}\n", _physicalDeviceProperties.limits.maxUniformBufferRange);
					fmt::print(stderr, "maxStorageBufferRange = {}\n", _physicalDeviceProperties.limits.maxStorageBufferRange);
					fmt::print(stderr, "maxPushConstantsSize = {}\n", _physicalDeviceProperties.limits.maxPushConstantsSize);
					fmt::print(stderr, "maxMemoryAllocationCount = {}\n", _physicalDeviceProperties.limits.maxMemoryAllocationCount);
					fmt::print(stderr, "maxSamplerAllocationCount = {}\n", _physicalDeviceProperties.limits.maxSamplerAllocationCount);
					fmt::print(stderr, "bufferImageGranularity = {}\n", _physicalDeviceProperties.limits.bufferImageGranularity);
					fmt::print(stderr, "sparseAddressSpaceSize = {}\n", _physicalDeviceProperties.limits.sparseAddressSpaceSize);
					fmt::print(stderr, "maxBoundDescriptorSets = {}\n", _physicalDeviceProperties.limits.maxBoundDescriptorSets);
					fmt::print(stderr, "maxPerStageDescriptorSamplers = {}\n", _physicalDeviceProperties.limits.maxPerStageDescriptorSamplers);
					fmt::print(stderr, "maxPerStageDescriptorUniformBuffers = {}\n", _physicalDeviceProperties.limits.maxPerStageDescriptorUniformBuffers);
					fmt::print(stderr, "maxPerStageDescriptorStorageBuffers = {}\n", _physicalDeviceProperties.limits.maxPerStageDescriptorStorageBuffers);
					fmt::print(stderr, "maxPerStageDescriptorSampledImages = {}\n", _physicalDeviceProperties.limits.maxPerStageDescriptorSampledImages);
					fmt::print(stderr, "maxPerStageDescriptorStorageImages = {}\n", _physicalDeviceProperties.limits.maxPerStageDescriptorStorageImages);
					fmt::print(stderr, "maxPerStageDescriptorInputAttachments = {}\n", _physicalDeviceProperties.limits.maxPerStageDescriptorInputAttachments);
					fmt::print(stderr, "maxPerStageResources = {}\n", _physicalDeviceProperties.limits.maxPerStageResources);
					fmt::print(stderr, "maxDescriptorSetSamplers = {}\n", _physicalDeviceProperties.limits.maxDescriptorSetSamplers);
					fmt::print(stderr, "maxDescriptorSetUniformBuffers = {}\n", _physicalDeviceProperties.limits.maxDescriptorSetUniformBuffers);
					fmt::print(stderr, "maxDescriptorSetUniformBuffersDynamic = {}\n", _physicalDeviceProperties.limits.maxDescriptorSetUniformBuffersDynamic);
					fmt::print(stderr, "maxDescriptorSetStorageBuffers = {}\n", _physicalDeviceProperties.limits.maxDescriptorSetStorageBuffers);
					fmt::print(stderr, "maxDescriptorSetStorageBuffersDynamic = {}\n", _physicalDeviceProperties.limits.maxDescriptorSetStorageBuffersDynamic);
					fmt::print(stderr, "maxDescriptorSetSampledImages = {}\n", _physicalDeviceProperties.limits.maxDescriptorSetSampledImages);
					fmt::print(stderr, "maxDescriptorSetStorageImages = {}\n", _physicalDeviceProperties.limits.maxDescriptorSetStorageImages);
					fmt::print(stderr, "maxDescriptorSetInputAttachments = {}\n", _physicalDeviceProperties.limits.maxDescriptorSetInputAttachments);
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

	void VulkanContext::createAllocator()
	{
		const VmaAllocatorCreateInfo createInfo{
			.flags = 0,
			.physicalDevice = _physicalDevice,
			.device = _device,
			.preferredLargeHeapBlockSize = 0,
			.pAllocationCallbacks = nullptr,
			.pDeviceMemoryCallbacks = nullptr,
			.pHeapSizeLimit = nullptr,
			.pVulkanFunctions = nullptr,
			.instance = _instance,
			.vulkanApiVersion = VK_API_VERSION_1_0,
			.pTypeExternalMemoryHandleTypes = nullptr,
		};
		SEIR_VK(vmaCreateAllocator(&createInfo, &_allocator));
	}

	void VulkanContext::createCommandPool()
	{
		const VkCommandPoolCreateInfo createInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.queueFamilyIndex = _graphicsQueueFamily,
		};
		SEIR_VK(vkCreateCommandPool(_device, &createInfo, nullptr, &_commandPool));
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

	void VulkanContext::copyBuffer(VkBuffer dst, VkBuffer src, VkDeviceSize size) const
	{
		auto commandBuffer = createCommandBuffer(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		const VkBufferCopy region{
			.srcOffset = 0,
			.dstOffset = 0,
			.size = size,
		};
		vkCmdCopyBuffer(commandBuffer, src, dst, 1, &region);
		commandBuffer.finishAndSubmit(_graphicsQueue);
	}
}
