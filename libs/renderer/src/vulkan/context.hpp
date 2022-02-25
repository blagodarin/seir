// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/shared_ptr.hpp>
#include "options.hpp"
#include "vulkan.hpp"

#include <array>
#include <cassert>
#include <vector>

namespace seir
{
	struct Size2D;
	class VulkanContext;
	class VulkanPipeline;
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
		constexpr VulkanBuffer() noexcept = default;
		constexpr VulkanBuffer(VulkanBuffer&&) noexcept;
		VulkanBuffer& operator=(VulkanBuffer&&) noexcept;
		~VulkanBuffer() noexcept { destroy(); }

		void destroy() noexcept;
		[[nodiscard]] constexpr VkBuffer handle() const noexcept { return _buffer; }
		void write(const void* data, VkDeviceSize size, VkDeviceSize offset = 0);

	private:
		VkDevice _device = VK_NULL_HANDLE;
		VkBuffer _buffer = VK_NULL_HANDLE;
		VkDeviceMemory _memory = VK_NULL_HANDLE;
		friend VulkanContext;
	};

	class VulkanImage
	{
	public:
		constexpr VulkanImage() noexcept = default;
		constexpr VulkanImage(VulkanImage&&) noexcept;
		VulkanImage& operator=(VulkanImage&&) noexcept;
		~VulkanImage() noexcept { destroy(); }

		void copy2D(const VulkanContext&, VkBuffer, const VkExtent2D&, uint32_t pixelStride);
		void destroy() noexcept;
		[[nodiscard]] constexpr VkFormat format() const noexcept { return _format; }
		[[nodiscard]] constexpr VkImage handle() const noexcept { return _image; }
		void transitionLayout(const VulkanContext&, VkImageLayout oldLayout, VkImageLayout newLayout);
		[[nodiscard]] constexpr VkImageView viewHandle() const noexcept { return _view; }

	private:
		VkDevice _device = VK_NULL_HANDLE;
		VkImage _image = VK_NULL_HANDLE;
		VkDeviceMemory _memory = VK_NULL_HANDLE;
		VkImageView _view = VK_NULL_HANDLE;
		VkFormat _format = VK_FORMAT_UNDEFINED;
		constexpr VulkanImage(VkDevice device, VkFormat format) noexcept
			: _device{ device }, _format{ format } {}
		friend VulkanContext;
	};

	class VulkanShader
	{
	public:
		constexpr VulkanShader() noexcept = default;
		constexpr VulkanShader(VulkanShader&&) noexcept;
		VulkanShader& operator=(VulkanShader&&) noexcept;
		~VulkanShader() noexcept { destroy(); }

		void destroy() noexcept;
		[[nodiscard]] constexpr VkShaderModule handle() const noexcept { return _module; }

	private:
		VkDevice _device = VK_NULL_HANDLE;
		VkShaderModule _module = VK_NULL_HANDLE;
		friend VulkanContext;
	};

	class VulkanRenderTarget
	{
	public:
		VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
		VkExtent2D _swapchainExtent{};
		std::vector<VkImage> _swapchainImages;
		std::vector<VkImageView> _swapchainImageViews;
		VulkanImage _colorBuffer;
		VulkanImage _depthBuffer;
		VkRenderPass _renderPass = VK_NULL_HANDLE;
		std::vector<VkFramebuffer> _swapchainFramebuffers;
		std::vector<VkFence> _swapchainImageFences;

		void create(const VulkanContext&, const Size2D& windowSize);
		void destroy(VkDevice) noexcept;

	private:
		void createSwapchain(const VulkanContext&, const Size2D& windowSize);
		void createSwapchainImageViews(VkDevice, const VkSurfaceFormatKHR&);
		void createColorBuffer(const VulkanContext&);
		void createDepthBuffer(const VulkanContext&);
		void createRenderPass(VkDevice, VkFormat, VkSampleCountFlagBits);
		void createFramebuffers(VkDevice);
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
		const RendererOptions _options;
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
		VkSampler _textureSampler = VK_NULL_HANDLE;

		explicit VulkanContext(const RendererOptions& options) noexcept
			: _options{ options } {}
		~VulkanContext() noexcept;

		void create(const WindowDescriptor&);
		VulkanBuffer createBuffer(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags) const;
		VulkanBuffer createDeviceBuffer(const void* data, VkDeviceSize, VkBufferUsageFlags) const;
		VulkanImage createImage2D(const VkExtent2D&, VkFormat, VkSampleCountFlagBits, VkImageTiling, VkImageUsageFlags, VkImageAspectFlags) const;
		VulkanShader createShader(const uint32_t* data, size_t bytes) const;
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
		void createTextureImage(const VkExtent2D&, VkFormat, VkDeviceSize, const void* data, uint32_t pixelStride);
		void copyBuffer(VkBuffer dst, VkBuffer src, VkDeviceSize) const;
	};

	class VulkanSwapchain // Actually "VulkanContextPart2".
	{
	public:
		std::vector<VkCommandBuffer> _commandBuffers;
		std::vector<VulkanBuffer> _uniformBuffers;
		VkDescriptorPool _descriptorPool = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> _descriptorSets;

		void create(const VulkanContext&, const VulkanRenderTarget&, const VulkanPipeline&, VkBuffer vertexBuffer, VkBuffer indexBuffer, uint32_t indexCount);
		void destroy(VkDevice, VkCommandPool) noexcept;
		void updateUniformBuffer(uint32_t imageIndex, const VkExtent2D& screenSize);

	private:
		void createUniformBuffers(const VulkanContext&, uint32_t count);
		void createDescriptorPool(VkDevice, uint32_t count);
		void createDescriptorSets(const VulkanContext&, VkDescriptorSetLayout, uint32_t count);
		void createCommandBuffers(VkDevice, VkCommandPool, const VulkanRenderTarget&, const VulkanPipeline&, VkBuffer vertexBuffer, VkBuffer indexBuffer, uint32_t indexCount);
	};
}

constexpr seir::VulkanBuffer::VulkanBuffer(VulkanBuffer&& other) noexcept
	: _device{ other._device }
	, _buffer{ other._buffer }
	, _memory{ other._memory }
{
	other._device = VK_NULL_HANDLE;
	other._buffer = VK_NULL_HANDLE;
	other._memory = VK_NULL_HANDLE;
}

constexpr seir::VulkanImage::VulkanImage(VulkanImage&& other) noexcept
	: _device{ other._device }
	, _image{ other._image }
	, _memory{ other._memory }
	, _view{ other._view }
	, _format{ other._format }
{
	other._device = VK_NULL_HANDLE;
	other._image = VK_NULL_HANDLE;
	other._memory = VK_NULL_HANDLE;
	other._view = VK_NULL_HANDLE;
	other._format = VK_FORMAT_UNDEFINED;
}

constexpr seir::VulkanShader::VulkanShader(VulkanShader&& other) noexcept
	: _device{ other._device }
	, _module{ other._module }
{
	other._device = VK_NULL_HANDLE;
	other._module = VK_NULL_HANDLE;
}
