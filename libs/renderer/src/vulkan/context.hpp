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

#include <vk_mem_alloc.h>

namespace seir::vulkan
{
	class CommandBuffer;
}

namespace seir
{
	class Size;
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

		void destroy(VkDevice) noexcept;
		void resize(VkDevice, uint32_t);
		Item switchFrame(VkDevice);

	private:
		size_t _index = 0;
		std::vector<Item> _items;
	};

	class VulkanBuffer
	{
	public:
		constexpr VulkanBuffer() noexcept = default;
		constexpr VulkanBuffer(VulkanBuffer&&) noexcept;
		VulkanBuffer& operator=(VulkanBuffer&&) noexcept;
		~VulkanBuffer() noexcept { destroy(); }

		[[nodiscard]] constexpr VmaAllocation allocation() const noexcept { return _allocation; }
		void destroy() noexcept;
		[[nodiscard]] constexpr VkBuffer handle() const noexcept { return _buffer; }
		[[nodiscard]] void* map();
		void unmap() noexcept { vmaUnmapMemory(_allocator, _allocation); }
		void write(const void* data, VkDeviceSize size);

	private:
		VmaAllocator _allocator = VK_NULL_HANDLE;
		VkBuffer _buffer = VK_NULL_HANDLE;
		VmaAllocation _allocation = VK_NULL_HANDLE;
		constexpr explicit VulkanBuffer(VmaAllocator allocator) noexcept
			: _allocator{ allocator } {}
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
		VmaAllocator _allocator = VK_NULL_HANDLE;
		VkImage _image = VK_NULL_HANDLE;
		VmaAllocation _allocation = VK_NULL_HANDLE;
		VkDevice _device = VK_NULL_HANDLE;
		VkImageView _view = VK_NULL_HANDLE;
		VkFormat _format = VK_FORMAT_UNDEFINED;
		constexpr VulkanImage(VmaAllocator allocator, VkDevice device, VkFormat format) noexcept
			: _allocator{ allocator }, _device{ device }, _format{ format } {}
		friend VulkanContext;
	};

	class VulkanSampler
	{
	public:
		constexpr VulkanSampler() noexcept = default;
		constexpr VulkanSampler(VulkanSampler&&) noexcept;
		VulkanSampler& operator=(VulkanSampler&&) noexcept;
		~VulkanSampler() noexcept { destroy(); }

		void destroy() noexcept;
		[[nodiscard]] constexpr VkSampler handle() const noexcept { return _sampler; }

	private:
		VkDevice _device = VK_NULL_HANDLE;
		VkSampler _sampler = VK_NULL_HANDLE;
		constexpr explicit VulkanSampler(VkDevice device) noexcept
			: _device{ device } {}
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
		constexpr explicit VulkanShader(VkDevice device) noexcept
			: _device{ device } {}
		friend VulkanContext;
	};

	class VulkanUniformBuffers
	{
	public:
		constexpr VulkanUniformBuffers() noexcept = default;
		constexpr VulkanUniformBuffers(VulkanUniformBuffers&& other) noexcept
			: _bufferSize{ other._bufferSize }, _buffers{ std::move(other._buffers) } {}
		VulkanUniformBuffers& operator=(VulkanUniformBuffers&&) noexcept;
		~VulkanUniformBuffers() noexcept { destroy(); }

		void destroy() noexcept;
		void update(size_t index, const void* data);

		[[nodiscard]] VkDescriptorBufferInfo operator[](size_t index) const noexcept { return { .buffer = _buffers[index].handle(), .offset = 0, .range = _bufferSize }; }

	private:
		VkDeviceSize _bufferSize = 0;
		std::vector<VulkanBuffer> _buffers;
		constexpr explicit VulkanUniformBuffers(VkDeviceSize size) noexcept
			: _bufferSize{ size } {}
		friend VulkanContext;
	};

	class VulkanRenderTarget
	{
	public:
		constexpr explicit operator bool() const noexcept { return _swapchain != VK_NULL_HANDLE; }

		bool acquireFrame(VkDevice, VkSemaphore signalSemaphore, VkFence waitFence, uint32_t& index);
		void create(const VulkanContext&, const Size& windowSize);
		void destroy(VkDevice) noexcept;
		[[nodiscard]] constexpr VkExtent2D extent() const noexcept { return _swapchainExtent; }
		[[nodiscard]] uint32_t frameCount() const noexcept { return static_cast<uint32_t>(_swapchainImages.size()); }
		bool presentFrame(VkQueue, uint32_t frameIndex, VkSemaphore waitSemaphore);
		[[nodiscard]] constexpr VkRenderPass renderPass() const noexcept { return _renderPass; }
		[[nodiscard]] VkRenderPassBeginInfo renderPassInfo(size_t frameIndex) const noexcept;

	private:
		void createSwapchain(const VulkanContext&, const Size& windowSize);
		void createSwapchainImageViews(VkDevice, const VkSurfaceFormatKHR&);
		void createColorBuffer(const VulkanContext&);
		void createDepthBuffer(const VulkanContext&);
		void createRenderPass(VkDevice, VkFormat, VkSampleCountFlagBits);
		void createFramebuffers(VkDevice);

	private:
		VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
		VkExtent2D _swapchainExtent{};
		std::vector<VkImage> _swapchainImages;
		std::vector<VkImageView> _swapchainImageViews;
		VulkanImage _colorBuffer;
		VulkanImage _depthBuffer;
		VkRenderPass _renderPass = VK_NULL_HANDLE;
		std::vector<VkFramebuffer> _swapchainFramebuffers;
		std::vector<VkFence> _swapchainImageFences;
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
		VmaAllocator _allocator = VK_NULL_HANDLE;
		VkCommandPool _commandPool = VK_NULL_HANDLE;

		explicit VulkanContext(const RendererOptions& options) noexcept
			: _options{ options } {}
		~VulkanContext() noexcept;

		void create(const WindowDescriptor&);
		[[nodiscard]] VulkanBuffer createBuffer(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags, VmaAllocationCreateFlags) const;
		[[nodiscard]] vulkan::CommandBuffer createCommandBuffer(VkCommandBufferUsageFlags) const;
		[[nodiscard]] VulkanBuffer createDeviceBuffer(const void* data, VkDeviceSize, VkBufferUsageFlags) const;
		[[nodiscard]] VulkanImage createImage2D(const VkExtent2D&, VkFormat, VkSampleCountFlagBits, VkImageTiling, VkImageUsageFlags, VkImageAspectFlags) const;
		[[nodiscard]] VulkanSampler createSampler2D() const;
		[[nodiscard]] VulkanShader createShader(const uint32_t* data, size_t bytes) const;
		[[nodiscard]] VulkanImage createTextureImage2D(const VkExtent2D&, VkFormat, VkDeviceSize, const void* data, uint32_t pixelStride) const;
		[[nodiscard]] VulkanUniformBuffers createUniformBuffers(VkDeviceSize size, size_t count) const;
		[[nodiscard]] uint32_t findMemoryType(uint32_t filter, VkMemoryPropertyFlags properties) const;
		[[nodiscard]] VkFormat findFormat(const std::vector<VkFormat>& candidates, VkImageTiling, VkFormatFeatureFlags) const;

	private:
		void createInstance();
#ifndef NDEBUG
		void createDebugUtilsMessenger();
#endif
		void createSurface(const WindowDescriptor&);
		void selectPhysicalDevice();
		void createDevice();
		void createCommandPool();
		void createAllocator();
		void copyBuffer(VkBuffer dst, VkBuffer src, VkDeviceSize) const;
	};
}

constexpr seir::VulkanBuffer::VulkanBuffer(VulkanBuffer&& other) noexcept
	: _allocator{ other._allocator }
	, _buffer{ other._buffer }
	, _allocation{ other._allocation }
{
	other._allocator = VK_NULL_HANDLE;
	other._buffer = VK_NULL_HANDLE;
	other._allocation = VK_NULL_HANDLE;
}

constexpr seir::VulkanImage::VulkanImage(VulkanImage&& other) noexcept
	: _allocator{ other._allocator }
	, _image{ other._image }
	, _allocation{ other._allocation }
	, _device{ other._device }
	, _view{ other._view }
	, _format{ other._format }
{
	other._allocator = VK_NULL_HANDLE;
	other._image = VK_NULL_HANDLE;
	other._allocation = VK_NULL_HANDLE;
	other._device = VK_NULL_HANDLE;
	other._view = VK_NULL_HANDLE;
	other._format = VK_FORMAT_UNDEFINED;
}

constexpr seir::VulkanSampler::VulkanSampler(VulkanSampler&& other) noexcept
	: _device{ other._device }
	, _sampler{ other._sampler }
{
	other._device = VK_NULL_HANDLE;
	other._sampler = VK_NULL_HANDLE;
}

constexpr seir::VulkanShader::VulkanShader(VulkanShader&& other) noexcept
	: _device{ other._device }
	, _module{ other._module }
{
	other._device = VK_NULL_HANDLE;
	other._module = VK_NULL_HANDLE;
}
