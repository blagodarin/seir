// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/shared_ptr.hpp>
#include "pipeline.hpp"

#include <array>
#include <cassert>
#include <vector>

namespace seir
{
	struct Size2D;
	class VulkanContext;
	struct WindowDescriptor;

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

	class VulkanBuffer
	{
	public:
		VkBuffer _buffer = VK_NULL_HANDLE;
		VkDeviceMemory _memory = VK_NULL_HANDLE;

		void create(const VulkanContext&, VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags);
		void destroy(VkDevice) noexcept;
		void write(VkDevice, const void* data, VkDeviceSize size, VkDeviceSize offset = 0);
	};

	class VulkanImage
	{
	public:
		VkImage _image = VK_NULL_HANDLE;
		VkDeviceMemory _memory = VK_NULL_HANDLE;

		void copy2D(const VulkanContext&, VkBuffer, uint32_t width, uint32_t height);
		void createTexture2D(const VulkanContext&, const VkExtent2D&, VkFormat, VkSampleCountFlagBits, VkImageTiling, VkImageUsageFlags);
		void destroy(VkDevice) noexcept;
		void transitionLayout(const VulkanContext&, VkFormat, VkImageLayout oldLayout, VkImageLayout newLayout);
	};

	class VulkanSwapchain
	{
	public:
		VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
		VkExtent2D _swapchainExtent{};
		std::vector<VkImage> _swapchainImages;
		std::vector<VkImageView> _swapchainImageViews;
		VulkanImage _colorBuffer;
		VkImageView _colorBufferView = VK_NULL_HANDLE;
		VkFormat _depthBufferFormat = VK_FORMAT_UNDEFINED;
		VulkanImage _depthBuffer;
		VkImageView _depthBufferView = VK_NULL_HANDLE;
		VkRenderPass _renderPass = VK_NULL_HANDLE;
		VulkanPipeline _pipeline;
		std::vector<VkFramebuffer> _swapchainFramebuffers;
		std::vector<VkCommandBuffer> _commandBuffers;
		std::vector<VulkanBuffer> _uniformBuffers;
		VkDescriptorPool _descriptorPool = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> _descriptorSets;
		std::vector<VkFence> _swapchainImageFences;

		void create(const VulkanContext&, const Size2D& windowSize);
		void destroy(VkDevice, VkCommandPool) noexcept;
		void updateUniformBuffer(VkDevice, uint32_t imageIndex);

	private:
		void createSwapchain(const VulkanContext&, const Size2D& windowSize);
		void createSwapchainImageViews(VkDevice, const VkSurfaceFormatKHR&);
		void createColorBuffer(const VulkanContext&);
		void createDepthBuffer(const VulkanContext&);
		void createRenderPass(VkDevice, VkFormat, VkSampleCountFlagBits);
		void createPipeline(VkDevice, VkSampleCountFlagBits, VkShaderModule vertexShader, VkShaderModule fragmentShader);
		void createFramebuffers(VkDevice);
		void createUniformBuffers(const VulkanContext&);
		void createDescriptorPool(VkDevice);
		void createDescriptorSets(const VulkanContext&);
		void createCommandBuffers(VkDevice, VkCommandPool, VkBuffer vertexBuffer, VkBuffer indexBuffer);
	};

	class VulkanOneTimeSubmit
	{
	public:
		VulkanOneTimeSubmit(VkDevice, VkCommandPool);
		~VulkanOneTimeSubmit() noexcept;

		[[nodiscard]] constexpr operator VkCommandBuffer() noexcept { return _commandBuffer; }
		void submit(VkQueue);

	private:
		const VkDevice _device;
		const VkCommandPool _commandPool;
		VkCommandBuffer _commandBuffer = VK_NULL_HANDLE;
	};

	class VulkanContext
	{
	public:
		const bool _useAnisotropy;
		const bool _useMsaa; // TODO: Support different MSAA levels.
		VkInstance _instance = VK_NULL_HANDLE;
#ifndef NDEBUG
		PFN_vkDestroyDebugUtilsMessengerEXT _vkDestroyDebugUtilsMessenger = nullptr;
		VkDebugUtilsMessengerEXT _debugUtilsMessenger = VK_NULL_HANDLE;
#endif
		VkSurfaceKHR _surface = VK_NULL_HANDLE;
		VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
		VkPhysicalDeviceProperties _physicalDeviceProperties{};
		VkSurfaceFormatKHR _surfaceFormat{};
		VkPresentModeKHR _presentMode = VK_PRESENT_MODE_FIFO_KHR;
		uint32_t _graphicsQueueFamily = 0;
		uint32_t _presentQueueFamily = 0;
		VkSampleCountFlagBits _maxSampleCount = VK_SAMPLE_COUNT_1_BIT;
		VkDevice _device = VK_NULL_HANDLE;
		VkQueue _graphicsQueue = VK_NULL_HANDLE;
		VkQueue _presentQueue = VK_NULL_HANDLE;
		VkCommandPool _commandPool = VK_NULL_HANDLE;
		VulkanImage _texture;
		VkImageView _textureView = VK_NULL_HANDLE;
		VkSampler _textureSampler = VK_NULL_HANDLE;
		VkShaderModule _vertexShader = VK_NULL_HANDLE;
		VkShaderModule _fragmentShader = VK_NULL_HANDLE;
		VulkanBuffer _vertexBuffer;
		VulkanBuffer _indexBuffer;

		VulkanContext(bool useAnisotropy, bool useMsaa) noexcept
			: _useAnisotropy{ useAnisotropy }, _useMsaa{ useMsaa } {}
		~VulkanContext() noexcept;

		void create(const WindowDescriptor&);
		uint32_t findMemoryType(uint32_t filter, VkMemoryPropertyFlags properties) const;
		VkFormat findFormat(const std::vector<VkFormat>& candidates, VkImageTiling, VkFormatFeatureFlags) const;

	private:
		void createInstance();
#ifndef NDEBUG
		void createDebugUtilsMessenger();
#endif
		void createSurface(const WindowDescriptor&);
		void selectPhysicalDevice();
		void createDevice();
		void createCommandPool();
		void createTextureImage();
		VkShaderModule loadShader(const uint32_t* data, size_t size);
		void createVertexBuffer();
		void createIndexBuffer();
		void copyBuffer(VkBuffer dst, VkBuffer src, VkDeviceSize);
	};
}
