// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/shared_ptr.hpp>

#include <array>
#include <cassert>
#include <vector>

#ifndef NDEBUG
#	include <string>
#endif

#include "vulkan.hpp"

namespace seir
{
	struct Size2D;
	class VulkanContext;
	class Window;
	struct WindowDescriptor;

	struct VulkanError
	{
#ifndef NDEBUG
		const std::string_view _function;
		const std::string _message;
		VulkanError(std::string_view function, std::string&& message) noexcept
			: _function{ function.substr(0, function.find('(')) }, _message{ std::move(message) } {}
		VulkanError(std::string_view function, VkResult status)
			: VulkanError(function, std::to_string(status)) {}
#endif
	};

#ifndef NDEBUG
#	define SEIR_VK_THROW(call, message) \
		throw seir::VulkanError(call, message)
#else
#	define SEIR_VK_THROW(call, message) \
		throw seir::VulkanError()
#endif

#define SEIR_VK(call) \
	do \
	{ \
		if (const auto status = (call); status != VK_SUCCESS) \
		{ \
			assert(!#call); \
			SEIR_VK_THROW(#call, std::to_string(status)); \
		} \
	} while (false)

	class VulkanFrameSync
	{
	public:
		struct Item
		{
			VkSemaphore _imageAvailableSemaphore = VK_NULL_HANDLE;
			VkSemaphore _renderFinishedSemaphore = VK_NULL_HANDLE;
			VkFence _fence = VK_NULL_HANDLE;
		};

		void create(VkDevice);
		void destroy(VkDevice) noexcept;
		Item switchFrame(VkDevice);

	private:
		size_t _index = 0;
		std::array<Item, 2> _items;
	};

	class VulkanSwapchain
	{
	public:
		VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
		VkExtent2D _swapchainExtent{};
		std::vector<VkImage> _swapchainImages;
		std::vector<VkImageView> _swapchainImageViews;
		VkRenderPass _renderPass = VK_NULL_HANDLE;
		VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
		VkPipeline _pipeline = VK_NULL_HANDLE;
		std::vector<VkFramebuffer> _swapchainFramebuffers;
		std::vector<VkCommandBuffer> _commandBuffers;
		std::vector<VkFence> _swapchainImageFences;

		void create(const VulkanContext&, const Size2D& windowSize);
		void destroy(VkDevice, VkCommandPool) noexcept;

	private:
		void createSwapchain(const VulkanContext&, const Size2D& windowSize);
		void createSwapchainImageViews(VkDevice, const VkSurfaceFormatKHR&);
		void createRenderPass(VkDevice, const VkSurfaceFormatKHR&);
		void createPipelineLayout(VkDevice);
		void createPipeline(VkDevice, VkShaderModule vertexShader, VkShaderModule fragmentShader);
		void createFramebuffers(VkDevice);
		void createCommandBuffers(VkDevice, VkCommandPool, VkBuffer vertexBuffer);
	};

	class VulkanContext
	{
	public:
		VkInstance _instance = VK_NULL_HANDLE;
#ifndef NDEBUG
		PFN_vkDestroyDebugUtilsMessengerEXT _vkDestroyDebugUtilsMessenger = nullptr;
		VkDebugUtilsMessengerEXT _debugUtilsMessenger = VK_NULL_HANDLE;
#endif
		VkSurfaceKHR _surface = VK_NULL_HANDLE;
		VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
		VkSurfaceFormatKHR _surfaceFormat{};
		VkPresentModeKHR _presentMode = VK_PRESENT_MODE_FIFO_KHR;
		uint32_t _graphicsQueueFamily = 0;
		uint32_t _presentQueueFamily = 0;
		VkDevice _device = VK_NULL_HANDLE;
		VkQueue _graphicsQueue = VK_NULL_HANDLE;
		VkQueue _presentQueue = VK_NULL_HANDLE;
		VkShaderModule _vertexShader = VK_NULL_HANDLE;
		VkShaderModule _fragmentShader = VK_NULL_HANDLE;
		VkBuffer _vertexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory _vertexBufferMemory = VK_NULL_HANDLE;
		VkCommandPool _commandPool = VK_NULL_HANDLE;

		~VulkanContext() noexcept;

		void create(const WindowDescriptor&);

	private:
		void createInstance();
#ifndef NDEBUG
		void createDebugUtilsMessenger();
#endif
		void createSurface(const WindowDescriptor&);
		void selectPhysicalDevice();
		void createDevice();
		void createCommandPool();
		VkShaderModule loadShader(const uint32_t* data, size_t size);
		uint32_t findMemoryType(uint32_t filter, VkMemoryPropertyFlags properties);
		void createVertexBuffer();
	};
}
