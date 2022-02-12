// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_renderer/renderer.hpp>

#include <seir_app/window.hpp>
#include "context.hpp"

#include <thread>

#ifndef NDEBUG
#	include <iostream>
#endif

namespace
{
	class VulkanRenderer final : public seir::Renderer
	{
	public:
		explicit VulkanRenderer(const seir::SharedPtr<seir::Window>& window) noexcept
			: _window{ window } {}

		~VulkanRenderer() noexcept override
		{
			vkDeviceWaitIdle(_context._device);
			_swapchain.destroy(_context._device, _context._commandPool);
			_frameSync.destroy(_context._device);
		}

		bool initialize()
		{
			try
			{
				_context.create(_window->descriptor());
				_frameSync.create(_context._device);
				return true;
			}
			catch ([[maybe_unused]] const seir::VulkanError& e)
			{
#ifndef NDEBUG
				std::cerr << '[' << e._function << "] " << e._message << '\n';
#endif
				return false;
			}
		}

		void draw() override
		{
			if (!_swapchain._swapchain)
			{
				const auto windowSize = _window->size();
				if (windowSize._width == 0 || windowSize._height == 0)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds{ 1 });
					return;
				}
				_swapchain.create(_context, windowSize);
			}
			const auto [imageAvailableSemaphore, renderFinishedSemaphore, fence] = _frameSync.switchFrame(_context._device);
			uint32_t index = 0;
			if (const auto status = vkAcquireNextImageKHR(_context._device, _swapchain._swapchain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &index); status == VK_ERROR_OUT_OF_DATE_KHR)
			{
				vkDeviceWaitIdle(_context._device);
				_swapchain.destroy(_context._device, _context._commandPool);
				return;
			}
			else if (status != VK_SUCCESS && status != VK_SUBOPTIMAL_KHR)
				SEIR_VK_THROW("vkAcquireNextImageKHR", status);
			if (_swapchain._swapchainImageFences[index])
				SEIR_VK(vkWaitForFences(_context._device, 1, &_swapchain._swapchainImageFences[index], VK_TRUE, UINT64_MAX));
			_swapchain._swapchainImageFences[index] = fence;
			const VkSemaphore waitSemaphores[]{ imageAvailableSemaphore };
			const VkPipelineStageFlags waitStages[]{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			const VkSemaphore signalSemaphores[]{ renderFinishedSemaphore };
			const VkSubmitInfo submitInfo{
				.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
				.waitSemaphoreCount = 1,
				.pWaitSemaphores = waitSemaphores,
				.pWaitDstStageMask = waitStages,
				.commandBufferCount = 1,
				.pCommandBuffers = &_swapchain._commandBuffers[index],
				.signalSemaphoreCount = 1,
				.pSignalSemaphores = signalSemaphores,
			};
			SEIR_VK(vkResetFences(_context._device, 1, &fence));
			SEIR_VK(vkQueueSubmit(_context._graphicsQueue, 1, &submitInfo, fence));
			const VkSwapchainKHR swapchains[]{ _swapchain._swapchain };
			const VkPresentInfoKHR presentInfo{
				.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
				.waitSemaphoreCount = 1,
				.pWaitSemaphores = signalSemaphores,
				.swapchainCount = 1,
				.pSwapchains = swapchains,
				.pImageIndices = &index,
				.pResults = nullptr,
			};
			if (const auto status = vkQueuePresentKHR(_context._presentQueue, &presentInfo); status == VK_ERROR_OUT_OF_DATE_KHR || status == VK_SUBOPTIMAL_KHR)
			{
				vkDeviceWaitIdle(_context._device);
				_swapchain.destroy(_context._device, _context._commandPool);
				return;
			}
			else if (status != VK_SUCCESS)
				SEIR_VK_THROW("vkQueuePresentKHR", status);
			SEIR_VK(vkQueueWaitIdle(_context._presentQueue));
		}

	private:
		const seir::SharedPtr<seir::Window> _window;
		seir::VulkanContext _context;
		seir::VulkanFrameSync _frameSync;
		seir::VulkanSwapchain _swapchain;
	};
}

namespace seir
{
	UniquePtr<Renderer> Renderer::create(const SharedPtr<Window>& window)
	{
		if (auto renderer = makeUnique<VulkanRenderer>(window); renderer->initialize())
			return staticCast<Renderer>(std::move(renderer));
		return {};
	}
}
