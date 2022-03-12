// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "vulkan.hpp"

namespace seir
{
	class VulkanContext;
}

namespace seir::vulkan
{
	class CommandBuffer
	{
	public:
		constexpr CommandBuffer() noexcept = default;
		constexpr CommandBuffer(CommandBuffer&& other) noexcept
			: _device{ other._device }, _pool{ other._pool }, _buffer{ other._buffer } { other._buffer = VK_NULL_HANDLE; }
		~CommandBuffer() noexcept { destroy(); }

		[[nodiscard]] constexpr operator VkCommandBuffer() const noexcept { return _buffer; }
		void destroy() noexcept;
		void finish();
		void finishAndSubmit(VkQueue);
		void submit(VkQueue, VkSemaphore waitSemaphore, VkPipelineStageFlags waitFlags, VkSemaphore signalSemaphore, VkFence signalFence);

	private:
		VkDevice _device = VK_NULL_HANDLE;
		VkCommandPool _pool = VK_NULL_HANDLE;
		VkCommandBuffer _buffer = VK_NULL_HANDLE;
		constexpr CommandBuffer(VkDevice device, VkCommandPool pool) noexcept
			: _device{ device }, _pool{ pool } {}
		friend VulkanContext;
	};
}
