// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_base/shared_ptr.hpp>

#include <array>
#include <vector>

#include "vulkan.hpp"

namespace seir
{
	class Window;

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
		VkPresentModeKHR _presentMode{};
		uint32_t _graphicsQueueFamily = 0;
		uint32_t _presentQueueFamily = 0;
		VkDevice _device = VK_NULL_HANDLE;
		VkQueue _graphicsQueue = VK_NULL_HANDLE;
		VkQueue _presentQueue = VK_NULL_HANDLE;
		VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
		VkExtent2D _swapchainExtent{};
		std::vector<VkImage> _swapchainImages;
		std::vector<VkImageView> _swapchainImageViews;
		VkShaderModule _vertexShader = VK_NULL_HANDLE;
		VkShaderModule _fragmentShader = VK_NULL_HANDLE;
		VkRenderPass _renderPass = VK_NULL_HANDLE;
		VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
		VkPipeline _pipeline = VK_NULL_HANDLE;
		std::vector<VkFramebuffer> _swapchainFramebuffers;
		VkCommandPool _commandPool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> _commandBuffers;
		std::vector<VkFence> _swapchainImageFences;

		struct FrameSync
		{
			struct Item
			{
				VkSemaphore _imageAvailableSemaphore = VK_NULL_HANDLE;
				VkSemaphore _renderFinishedSemaphore = VK_NULL_HANDLE;
				VkFence _fence = VK_NULL_HANDLE;
			};

			std::array<Item, 2> _items;
			size_t _index = 0;

			void createObjects(VkDevice);
			void destroyObjects(VkDevice) noexcept;
			Item nextFrameObjects(VkDevice);
		};

		FrameSync _frameSync;

		VulkanContext(const SharedPtr<Window>& window) noexcept
			: _window{ window } {}
		~VulkanContext() noexcept;

		bool initialize();
		void draw();

	private:
		void createInstance();
#ifndef NDEBUG
		void createDebugUtilsMessenger();
#endif
		void createSurface(const Window&);
		void selectPhysicalDevice();
		void createDevice();
		void createSwapchain(const Window&);
		void createSwapchainImageViews();
		VkShaderModule loadShader(const uint32_t* data, size_t size);
		void createRenderPass();
		void createPipelineLayout();
		void createPipeline();
		void createFramebuffers();
		void createCommandPool();
		void createCommandBuffers();
		void createSwapchainObjects();
		void destroySwapchainObjects();

	private:
		const SharedPtr<Window> _window;
	};
}
