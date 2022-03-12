// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "descriptors.hpp"

#include "error.hpp"

namespace seir::vulkan
{
	DescriptorAllocator::~DescriptorAllocator() noexcept
	{
		for (const auto pool : _pools)
			vkDestroyDescriptorPool(_device, pool, nullptr);
	}

	VkDescriptorSet DescriptorAllocator::allocate(VkDescriptorSetLayout layout)
	{
		VkDescriptorSetAllocateInfo info{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = _pools[_poolIndex],
			.descriptorSetCount = 1,
			.pSetLayouts = &layout,
		};
		VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
		SEIR_VK(vkAllocateDescriptorSets(_device, &info, &descriptorSet));
		auto status = vkAllocateDescriptorSets(_device, &info, &descriptorSet);
		if (status == VK_ERROR_FRAGMENTED_POOL || status == VK_ERROR_OUT_OF_POOL_MEMORY)
		{
			_poolIndex += _frameCount;
			if (_poolIndex >= _pools.size())
				grow();
			info.descriptorPool = _pools[_poolIndex];
			status = vkAllocateDescriptorSets(_device, &info, &descriptorSet);
		}
		if (status != VK_SUCCESS)
			SEIR_VK_THROW("vkAllocateDescriptorSets", status);
		return descriptorSet;
	}

	void DescriptorAllocator::deallocateAll()
	{
		for (const auto pool : _pools)
			SEIR_VK(vkResetDescriptorPool(_device, pool, 0));
	}

	void DescriptorAllocator::reset(VkDevice device, uint32_t frameCount, uint32_t setsPerPool, std::vector<VkDescriptorPoolSize>&& poolSizes)
	{
		for (const auto pool : _pools)
			vkDestroyDescriptorPool(_device, pool, nullptr);
		_device = device;
		_frameCount = frameCount;
		_poolIndex = 0;
		_pools.clear();
		_setsPerPool = setsPerPool;
		_poolSizes = std::move(poolSizes);
		grow();
	}

	void DescriptorAllocator::setFrameIndex(uint32_t index)
	{
		_poolIndex = index;
		for (auto i = index; i < _pools.size(); i += _frameCount)
			SEIR_VK(vkResetDescriptorPool(_device, _pools[i], 0));
	}

	void DescriptorAllocator::grow()
	{
		const auto offset = _pools.size();
		_pools.resize(offset + _frameCount, VK_NULL_HANDLE);
		for (uint32_t i = 0; i < _frameCount; ++i)
		{
			const VkDescriptorPoolCreateInfo info{
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
				.maxSets = _setsPerPool,
				.poolSizeCount = static_cast<uint32_t>(_poolSizes.size()),
				.pPoolSizes = _poolSizes.data(),
			};
			SEIR_VK(vkCreateDescriptorPool(_device, &info, nullptr, &_pools[offset + i]));
		}
	}

	DescriptorBuilder& DescriptorBuilder::bindBuffer(uint32_t binding, VkDescriptorType type, const VkDescriptorBufferInfo& info) noexcept
	{
		_buffers.emplace_back(info);
		_writes.push_back({
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = VK_NULL_HANDLE,
			.dstBinding = binding,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = type,
			.pImageInfo = nullptr,
			.pBufferInfo = reinterpret_cast<const VkDescriptorBufferInfo*>(_buffers.size()),
			.pTexelBufferView = nullptr,
		});
		return *this;
	}

	DescriptorBuilder& DescriptorBuilder::bindImage(uint32_t binding, VkDescriptorType type, VkSampler sampler, VkImageView view, VkImageLayout layout) noexcept
	{
		_images.push_back({
			.sampler = sampler,
			.imageView = view,
			.imageLayout = layout,
		});
		_writes.push_back({
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = VK_NULL_HANDLE,
			.dstBinding = binding,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = type,
			.pImageInfo = reinterpret_cast<const VkDescriptorImageInfo*>(_images.size()),
			.pBufferInfo = nullptr,
			.pTexelBufferView = nullptr,
		});
		return *this;
	}

	VkDescriptorSet DescriptorBuilder::build(DescriptorAllocator& allocator, VkDescriptorSetLayout layout)
	{
		const auto descriptorSet = allocator.allocate(layout);
		for (auto& write : _writes)
		{
			write.dstSet = descriptorSet;
			if (write.pImageInfo)
				write.pImageInfo = &_images[reinterpret_cast<uintptr_t>(write.pImageInfo) - 1];
			if (write.pBufferInfo)
				write.pBufferInfo = &_buffers[reinterpret_cast<uintptr_t>(write.pBufferInfo) - 1];
		}
		vkUpdateDescriptorSets(allocator.device(), static_cast<uint32_t>(_writes.size()), _writes.data(), 0, nullptr);
		return descriptorSet;
	}
}
