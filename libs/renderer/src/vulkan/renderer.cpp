// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_renderer/renderer.hpp>

#include <seir_app/window.hpp>
#include <seir_base/pointer.hpp>
#include "vulkan.hpp"

#include <array>
#include <cassert>

#ifndef NDEBUG
#	include <iostream>
#	include <vector>
#endif

#define SEIR_VK(call) \
	do \
	{ \
		if (const auto status = (call)) \
		{ \
			assert(!#call); \
			return false; \
		} \
	} while (false)

namespace
{
#ifndef NDEBUG
	bool printVulkanLayers()
	{
		uint32_t count = 0;
		SEIR_VK(vkEnumerateInstanceLayerProperties(&count, nullptr));
		std::vector<VkLayerProperties> layers(count);
		SEIR_VK(vkEnumerateInstanceLayerProperties(&count, layers.data()));
		std::cerr << "Vulkan layers available:\n";
		for (const auto& layer : layers)
			std::cerr << '\t' << layer.layerName << " - " << layer.description << '\n';
		std::cerr << '\n';
		return true;
	}
#endif

	struct VulkanContext
	{
		VkInstance _instance = VK_NULL_HANDLE;
#ifndef NDEBUG
		PFN_vkDestroyDebugReportCallbackEXT _vkDestroyDebugReportCallback = nullptr;
		VkDebugReportCallbackEXT _debugReportCallback = VK_NULL_HANDLE;
#endif
		VkSurfaceKHR _surface = VK_NULL_HANDLE;

		~VulkanContext() noexcept
		{
			vkDestroySurfaceKHR(_instance, _surface, nullptr);
#ifndef NDEBUG
			if (_vkDestroyDebugReportCallback)
				_vkDestroyDebugReportCallback(_instance, _debugReportCallback, nullptr);
#endif
			vkDestroyInstance(_instance, nullptr);
		}

		bool createInstance() noexcept
		{
			assert(!_instance);

			VkApplicationInfo applicationInfo{};
			applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			applicationInfo.apiVersion = VK_API_VERSION_1_0;

			static const std::array extensions{
#ifndef NDEBUG
				VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
#endif
				VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_WIN32_KHR
				VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
			};

			VkInstanceCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			createInfo.pApplicationInfo = &applicationInfo;
			createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
			createInfo.ppEnabledExtensionNames = extensions.data();
			SEIR_VK(vkCreateInstance(&createInfo, nullptr, &_instance));
			return true;
		}

#ifndef NDEBUG
		bool createDebugReportCallback() noexcept
		{
			assert(_instance && !_debugReportCallback);
			const auto vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(_instance, "vkCreateDebugReportCallbackEXT"));
			const auto vkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(_instance, "vkDestroyDebugReportCallbackEXT"));
			if (!vkCreateDebugReportCallbackEXT || !vkDestroyDebugReportCallbackEXT)
				return false;
			VkDebugReportCallbackCreateInfoEXT createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
			createInfo.flags = VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT;
			createInfo.pfnCallback = [](VkDebugReportFlagsEXT, VkDebugReportObjectTypeEXT, uint64_t, size_t, int32_t, const char* layerPrefix, const char* message, void*) {
				std::cerr << '[' << layerPrefix << "] " << message << '\n';
				return VK_FALSE;
			};
			SEIR_VK(vkCreateDebugReportCallbackEXT(_instance, &createInfo, nullptr, &_debugReportCallback));
			_vkDestroyDebugReportCallback = vkDestroyDebugReportCallbackEXT;
			return true;
		}
#endif

		bool createSurface(const seir::Window& window) noexcept
		{
			assert(_instance && !_surface);
#ifdef VK_USE_PLATFORM_WIN32_KHR
			VkWin32SurfaceCreateInfoKHR createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
			const auto descriptor = window.descriptor();
			createInfo.hinstance = static_cast<HINSTANCE>(descriptor._app);
			createInfo.hwnd = reinterpret_cast<HWND>(descriptor._window);
			SEIR_VK(vkCreateWin32SurfaceKHR(_instance, &createInfo, nullptr, &_surface));
			return true;
#endif
		}
	};

	class VulkanRenderer final : public seir::Renderer
	{
	public:
		explicit VulkanRenderer(const seir::SharedPtr<seir::Window>& window) noexcept
			: _window{ window } {}

	private:
		const seir::SharedPtr<seir::Window> _window;
	};
}

namespace seir
{
	UniquePtr<Renderer> Renderer::create(const SharedPtr<Window>& window)
	{
#ifndef NDEBUG
		if (!::printVulkanLayers())
			return {};
#endif
		VulkanContext context;
		if (context.createInstance()
#ifndef NDEBUG
			&& context.createDebugReportCallback()
#endif
			&& context.createSurface(*window))
			return makeUnique<Renderer, VulkanRenderer>(window);
		return {};
	}
}
