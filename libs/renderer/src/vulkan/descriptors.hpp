// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/static_vector.hpp>

#include "vulkan.hpp"

#include <vector>

namespace seir::vulkan
{
	class DescriptorAllocator
	{
	public:
		constexpr DescriptorAllocator() noexcept = default;
		DescriptorAllocator(DescriptorAllocator&) = delete;
		DescriptorAllocator& operator=(DescriptorAllocator&) = delete;
		~DescriptorAllocator() noexcept;

		VkDescriptorSet allocate(VkDescriptorSetLayout);
		void deallocateAll();
		[[nodiscard]] constexpr VkDevice device() const noexcept { return _device; }
		void flip();
		void reset(VkDevice, uint32_t frameCount, uint32_t setsPerPool, std::vector<VkDescriptorPoolSize>&&);

	private:
		void grow();

	private:
		VkDevice _device = VK_NULL_HANDLE;
		uint32_t _frameCount = 0;
		uint32_t _frameIndex = 0;
		std::vector<VkDescriptorPool> _pools;
		uint32_t _poolIndex = 0;
		uint32_t _setsPerPool = 0;
		std::vector<VkDescriptorPoolSize> _poolSizes;
	};

	class DescriptorBuilder
	{
	public:
		DescriptorBuilder& bindBuffer(uint32_t binding, VkDescriptorType, const VkDescriptorBufferInfo&) noexcept;
		DescriptorBuilder& bindImage(uint32_t binding, VkDescriptorType, VkSampler, VkImageView, VkImageLayout) noexcept;
		VkDescriptorSet build(DescriptorAllocator&, VkDescriptorSetLayout);

	private:
		StaticVector<VkWriteDescriptorSet, 4> _writes;
		StaticVector<VkDescriptorImageInfo, 2> _images;
		StaticVector<VkDescriptorBufferInfo, 2> _buffers;
	};
}
