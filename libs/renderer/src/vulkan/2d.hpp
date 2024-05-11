// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "context.hpp"

namespace seir
{
	class ShaderSet;
	class VulkanRenderPass;
	class VulkanRenderer;

	class Vulkan2D
	{
	public:
		VkBuffer indexBuffer(uint32_t frameIndex) const noexcept { return _buffers[frameIndex]._indexBuffer.handle(); }
		void initialize(VulkanRenderer&);
		void resize(uint32_t);
		const SharedPtr<ShaderSet>& shaders() const noexcept { return _shaders; }
		void updateBuffers(VulkanContext&, uint32_t frameIndex, const void* vertexData, VkDeviceSize vertexDataSize, const void* indexData, VkDeviceSize indexDataSize);
		VkBuffer vertexBuffer(uint32_t frameIndex) const noexcept { return _buffers[frameIndex]._vertexBuffer.handle(); }

	private:
		struct Buffers
		{
			VulkanBuffer _vertexBuffer;
			VkDeviceSize _vertexBufferSize = 0;
			VulkanBuffer _indexBuffer;
			VkDeviceSize _indexBufferSize = 0;
		};

		SharedPtr<ShaderSet> _shaders;
		std::vector<Buffers> _buffers;
	};
}
