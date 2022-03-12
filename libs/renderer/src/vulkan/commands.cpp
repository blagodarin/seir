// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "commands.hpp"

#include "error.hpp"

namespace seir::vulkan
{
	void CommandBuffer::destroy() noexcept
	{
		if (_buffer)
		{
			vkFreeCommandBuffers(_device, _pool, 1, &_buffer);
			_buffer = VK_NULL_HANDLE;
		}
	}

	void CommandBuffer::finish()
	{
		SEIR_VK(vkEndCommandBuffer(_buffer));
	}

	void CommandBuffer::finishAndSubmit(VkQueue queue)
	{
		finish();
		VkSubmitInfo submitInfo{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.commandBufferCount = 1,
			.pCommandBuffers = &_buffer,
		};
		SEIR_VK(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
		SEIR_VK(vkQueueWaitIdle(queue));
	}

	void CommandBuffer::submit(VkQueue queue, VkSemaphore waitSemaphore, VkPipelineStageFlags waitFlags, VkSemaphore signalSemaphore, VkFence signalFence)
	{
		const VkSubmitInfo info{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &waitSemaphore,
			.pWaitDstStageMask = &waitFlags,
			.commandBufferCount = 1,
			.pCommandBuffers = &_buffer,
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &signalSemaphore,
		};
		SEIR_VK(vkQueueSubmit(queue, 1, &info, signalFence));
	}
}
