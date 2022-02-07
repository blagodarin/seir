// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "context.hpp"

#include <seir_app/window.hpp>
#include <seir_base/static_vector.hpp>

#include <algorithm>
#include <array>
#include <optional>
#include <unordered_set>

#ifndef NDEBUG
#	include <cassert>
#	include <iostream>
#endif

namespace
{
	struct VulkanError
	{
#ifndef NDEBUG
		const std::string_view _function;
		const std::string _message;
		VulkanError(std::string_view function, std::string&& message) noexcept
			: _function{ function.substr(0, function.find('(')) }, _message{ std::move(message) } {}
#endif
	};

#ifndef NDEBUG
#	define SEIR_VK_THROW(call, message) \
		throw VulkanError(call, message)
#else
#	define SEIR_VK_THROW(call, message) \
		throw VulkanError()
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
}

namespace seir
{
	VulkanContext::~VulkanContext() noexcept
	{
		for (auto imageView : _swapchainImageViews)
			vkDestroyImageView(_device, imageView, nullptr);
		vkDestroySwapchainKHR(_device, _swapchain, nullptr);
		vkDestroyDevice(_device, nullptr);
		vkDestroySurfaceKHR(_instance, _surface, nullptr);
#ifndef NDEBUG
		if (_vkDestroyDebugUtilsMessenger)
			_vkDestroyDebugUtilsMessenger(_instance, _debugUtilsMessenger, nullptr);
#endif
		vkDestroyInstance(_instance, nullptr);
	}

	bool VulkanContext::initialize(Window& window)
	{
		try
		{
			// This is not a constructor because we need the destructor to execute if initialization fails.
#ifndef NDEBUG
			::printInstanceInfo();
#endif
			createInstance();
#ifndef NDEBUG
			createDebugUtilsMessenger();
#endif
			createSurface(window);
			selectPhysicalDevice();
			createDevice();
			createSwapchain(window);
			createSwapchainImageViews();
			return true;
		}
		catch ([[maybe_unused]] const VulkanError& e)
		{
#ifndef NDEBUG
			std::cerr << '[' << e._function << "] " << e._message << '\n';
#endif
			return false;
		}
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

	void VulkanContext::createSurface(const Window& window)
	{
		const auto descriptor = window.descriptor();
#ifdef VK_USE_PLATFORM_WIN32_KHR
		const VkWin32SurfaceCreateInfoKHR createInfo{
			.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
			.hinstance = static_cast<HINSTANCE>(descriptor._app),
			.hwnd = reinterpret_cast<HWND>(descriptor._window),
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
		seir::StaticVector<VkDeviceQueueCreateInfo, 2> queues;
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

	void VulkanContext::createSwapchain(const Window& window)
	{
		VkSurfaceCapabilitiesKHR surfaceCapabilities{};
		SEIR_VK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, _surface, &surfaceCapabilities));

		_swapchainExtent = surfaceCapabilities.currentExtent;
		if (_swapchainExtent.width == std::numeric_limits<uint32_t>::max() || _swapchainExtent.height == std::numeric_limits<uint32_t>::max())
		{
			const auto windowSize = window.size();
			_swapchainExtent.width = std::clamp(static_cast<uint32_t>(windowSize._width), surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
			_swapchainExtent.height = std::clamp(static_cast<uint32_t>(windowSize._height), surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
		}

		auto imageCount = surfaceCapabilities.minImageCount + 1;
		if (surfaceCapabilities.maxImageCount > 0 && imageCount > surfaceCapabilities.maxImageCount)
			imageCount = surfaceCapabilities.maxImageCount;

		const std::array queueFamilies{ _graphicsQueueFamily, _presentQueueFamily };
		VkSwapchainCreateInfoKHR createInfo{
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.surface = _surface,
			.minImageCount = imageCount,
			.imageFormat = _surfaceFormat.format,
			.imageColorSpace = _surfaceFormat.colorSpace,
			.imageExtent = _swapchainExtent,
			.imageArrayLayers = 1,
			.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			.preTransform = surfaceCapabilities.currentTransform,
			.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			.presentMode = _presentMode,
			.clipped = VK_TRUE,
		};
		if (_graphicsQueueFamily != _presentQueueFamily)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilies.size());
			createInfo.pQueueFamilyIndices = queueFamilies.data();
		}
		else
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		SEIR_VK(vkCreateSwapchainKHR(_device, &createInfo, nullptr, &_swapchain));
		SEIR_VK(vkGetSwapchainImagesKHR(_device, _swapchain, &imageCount, nullptr));
		_swapchainImages.resize(imageCount);
		SEIR_VK(vkGetSwapchainImagesKHR(_device, _swapchain, &imageCount, _swapchainImages.data()));
	}

	void VulkanContext::createSwapchainImageViews()
	{
		_swapchainImageViews.assign(_swapchainImages.size(), VK_NULL_HANDLE);
		for (size_t i = 0; i < _swapchainImages.size(); ++i)
		{
			const VkImageViewCreateInfo createInfo{
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.image = _swapchainImages[i],
				.viewType = VK_IMAGE_VIEW_TYPE_2D,
				.format = _surfaceFormat.format,
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
			SEIR_VK(vkCreateImageView(_device, &createInfo, nullptr, &_swapchainImageViews[i]));
		}
	}
}
