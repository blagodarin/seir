// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "2d.hpp"

#include <seir_base/scope.hpp>
#include "error.hpp"
#include "renderer.hpp"

namespace
{
	const uint32_t kVertexShader[]{
#include "2d_vertex.glsl.spirv.inc"
	};

	const uint32_t kFragmentShader[]{
#include "2d_fragment.glsl.spirv.inc"
	};
}

namespace seir
{
	void Vulkan2D::initialize(RendererImpl& renderer)
	{
		assert(!_shaders);
		_shaders = renderer.createShaders(kVertexShader, kFragmentShader);
	}

	void Vulkan2D::resize(uint32_t frameCount)
	{
		if (frameCount > _buffers.size())
			_buffers.resize(frameCount);
	}

	void Vulkan2D::updateBuffers(VulkanContext& context, uint32_t frameIndex, const void* vertexData, VkDeviceSize vertexDataSize, const void* indexData, VkDeviceSize indexDataSize)
	{
		assert(frameIndex < _buffers.size());
		auto& buffers = _buffers[frameIndex];
		if (buffers._vertexBufferSize < vertexDataSize)
		{
			buffers._vertexBuffer.destroy();
			buffers._vertexBufferSize = 0;
			buffers._vertexBuffer = context.createBuffer(vertexDataSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
			buffers._vertexBufferSize = vertexDataSize;
		}
		if (buffers._indexBufferSize < indexDataSize)
		{
			buffers._indexBuffer.destroy();
			buffers._indexBufferSize = 0;
			buffers._indexBuffer = context.createBuffer(indexDataSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
			buffers._indexBufferSize = indexDataSize;
		}
		const auto vertices = buffers._vertexBuffer.map();
		SEIR_FINALLY([&buffers] { buffers._vertexBuffer.unmap(); });
		const auto indices = buffers._indexBuffer.map();
		SEIR_FINALLY([&buffers] { buffers._indexBuffer.unmap(); });
		std::memcpy(vertices, vertexData, vertexDataSize);
		std::memcpy(indices, indexData, indexDataSize);
		const VmaAllocation allocations[2]{ buffers._vertexBuffer.allocation(), buffers._indexBuffer.allocation() };
		const VkDeviceSize offsets[2]{ 0, 0 };
		const VkDeviceSize sizes[2]{ vertexDataSize, indexDataSize };
		SEIR_VK(vmaFlushAllocations(context._allocator, 2, allocations, offsets, sizes));
	}
}
