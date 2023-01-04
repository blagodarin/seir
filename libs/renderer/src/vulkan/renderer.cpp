// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "renderer.hpp"

#include <seir_app/window.hpp>
#include <seir_image/image.hpp>
#include <seir_math/euler.hpp>
#include <seir_math/mat.hpp>
#include "commands.hpp"
#include "error.hpp"
#include "utils.hpp"

#ifndef NDEBUG
#	include <fmt/core.h>
#endif

namespace
{
	struct UniformBufferObject
	{
		seir::Mat4 _model;
		seir::Mat4 _view;
		seir::Mat4 _projection;
	};

	const uint32_t kVertexShader[]{
#include "vertex_shader.glsl.spirv.inc"
	};

	const uint32_t kFragmentShader[]{
#include "fragment_shader.glsl.spirv.inc"
	};

	constexpr std::array<uint8_t, 16> kTextureData{
		0x99, 0xbb, 0xbb, 0xff,
		0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff
	};

	seir::VulkanPipeline createPipeline(const seir::VulkanContext& context, const seir::VulkanRenderTarget& renderTarget, VkShaderModule vertexShader, VkShaderModule fragmentShader)
	{
		seir::VulkanPipelineBuilder builder{ renderTarget.extent(), context._maxSampleCount, context._options.sampleShading };
		builder.setDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
		builder.setDescriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
		builder.setStage(VK_SHADER_STAGE_VERTEX_BIT, vertexShader);
		builder.setStage(VK_SHADER_STAGE_FRAGMENT_BIT, fragmentShader);
		builder.setVertexInput(0, { seir::VertexAttribute::f32x3, seir::VertexAttribute::f32x3, seir::VertexAttribute::f32x2 });
		builder.setInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, true);
		return builder.build(context._device, renderTarget.renderPass());
	}

	UniformBufferObject makeUniformBuffer(const VkExtent2D& screenSize)
	{
		return {
			._model = seir::Mat4::rotation(10 * seir::clockTime(), { 0, 0, 1 }),
			._view = seir::Mat4::camera({ 0, -3, 3 }, { 0, -45, 0 }),
			._projection = seir::Mat4::projection3D(static_cast<float>(screenSize.width) / static_cast<float>(screenSize.height), 45, 1),
		};
	}

	class VulkanMesh : public seir::Mesh
	{
	public:
		VulkanMesh(seir::VulkanBuffer&& vertexBuffer, seir::VulkanBuffer&& indexBuffer, VkIndexType indexType, uint32_t indexCount) noexcept
			: _vertexBuffer{ std::move(vertexBuffer) }, _indexBuffer{ std::move(indexBuffer) }, _indexType{ indexType }, _indexCount{ indexCount } {}
		constexpr auto indexCount() const noexcept { return _indexCount; }
		constexpr auto indexBufferHandle() const noexcept { return _indexBuffer.handle(); }
		constexpr auto indexType() const noexcept { return _indexType; }
		constexpr auto vertexBufferHandle() const noexcept { return _vertexBuffer.handle(); }

	private:
		const seir::VulkanBuffer _vertexBuffer;
		const seir::VulkanBuffer _indexBuffer;
		const VkIndexType _indexType;
		const uint32_t _indexCount;
	};

	class VulkanTexture2D : public seir::Texture2D
	{
	public:
		VulkanTexture2D(seir::VulkanImage&& image) noexcept
			: _image{ std::move(image) } {}
		constexpr auto viewHandle() const noexcept { return _image.viewHandle(); }

	private:
		seir::VulkanImage _image;
	};
}

namespace seir
{
	VulkanRenderer::VulkanRenderer(const seir::SharedPtr<seir::Window>& window) noexcept
		: _window{ window }
		, _context{
			RendererOptions{
				.anisotropicFiltering = true,
				.multisampleAntialiasing = true,
				.sampleShading = true,
			}
		}
	{
	}

	VulkanRenderer::~VulkanRenderer() noexcept
	{
		vkDeviceWaitIdle(_context._device);
		_uniformBuffers.destroy();
		_pipeline.destroy();
		_renderTarget.destroy(_context._device);
		_frameSync.destroy(_context._device);
	}

	bool VulkanRenderer::initialize()
	{
		try
		{
			_context.create(_window->descriptor());
			_vertexShader = _context.createShader(kVertexShader, sizeof kVertexShader);
			_fragmentShader = _context.createShader(kFragmentShader, sizeof kFragmentShader);
			_textureSampler = _context.createSampler2D();
			_frameSync.create(_context._device);
		}
		catch ([[maybe_unused]] const seir::VulkanError& e)
		{
#ifndef NDEBUG
			fmt::print(stderr, "[{}] {}\n", e._function, e._message);
#endif
			return false;
		}
		_whiteTexture2D = createTexture2D({ 1, 2, 8, PixelFormat::Bgra32 }, kTextureData.data());
		return static_cast<bool>(_whiteTexture2D);
	}

	UniquePtr<Mesh> VulkanRenderer::createMesh(const void* vertexData, size_t vertexSize, size_t vertexCount, const void* indexData, Mesh::IndexType indexType, size_t indexCount)
	{
		if (indexCount > std::numeric_limits<uint32_t>::max())
			return {};
		auto vulkanIndexType = VK_INDEX_TYPE_UINT16;
		size_t indexSize = 0;
		switch (indexType)
		{
		case Mesh::IndexType::U16:
			indexSize = sizeof(uint16_t);
			break;
		case Mesh::IndexType::U32:
			vulkanIndexType = VK_INDEX_TYPE_UINT32;
			indexSize = sizeof(uint32_t);
			break;
		}
		try
		{
			return makeUnique<Mesh, VulkanMesh>(_context.createDeviceBuffer(vertexData, vertexSize * vertexCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT),
				_context.createDeviceBuffer(indexData, indexSize * indexCount, VK_BUFFER_USAGE_INDEX_BUFFER_BIT), vulkanIndexType, static_cast<uint32_t>(indexCount));
		}
		catch ([[maybe_unused]] const seir::VulkanError& e)
		{
#ifndef NDEBUG
			fmt::print(stderr, "[{}] {}\n", e._function, e._message);
#endif
			return {};
		}
	}

	UniquePtr<Texture2D> VulkanRenderer::createTexture2D(const ImageInfo& info, const void* data)
	{
		if (info.pixelFormat() != PixelFormat::Bgra32)
			return {};
		const auto pixelSize = info.pixelSize();
		const auto stride = info.stride();
		if (stride % pixelSize)
			return {};
		try
		{
			return makeUnique<Texture2D, VulkanTexture2D>(_context.createTextureImage2D({ info.width(), info.height() }, VK_FORMAT_B8G8R8A8_SRGB, info.frameSize(), data, stride / pixelSize));
		}
		catch ([[maybe_unused]] const seir::VulkanError& e)
		{
#ifndef NDEBUG
			fmt::print(stderr, "[{}] {}\n", e._function, e._message);
#endif
			return {};
		}
	}

	void VulkanRenderer::draw(const Mesh& mesh)
	{
		if (!_renderTarget)
		{
			const auto windowSize = _window->size();
			if (windowSize._width == 0 || windowSize._height == 0)
			{
				sleepFor(1);
				return;
			}
			_renderTarget.create(_context, windowSize);
			_pipeline = ::createPipeline(_context, _renderTarget, _vertexShader.handle(), _fragmentShader.handle());
			_uniformBuffers = _context.createUniformBuffers(sizeof(UniformBufferObject), _renderTarget.frameCount());
			_descriptorAllocator.reset(_context._device, _renderTarget.frameCount(), 1'000,
				{
					VkDescriptorPoolSize{
						.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
						.descriptorCount = 1'000,
					},
					VkDescriptorPoolSize{
						.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
						.descriptorCount = 1'000,
					},
				});
		}
		const auto [frameAvailableSemaphore, frameRenderedSemaphore, frameFence] = _frameSync.switchFrame(_context._device);
		uint32_t index = 0;
		if (!_renderTarget.acquireFrame(_context._device, frameAvailableSemaphore, frameFence, index))
			return resetRenderTarget();
		{
			const auto ubo = ::makeUniformBuffer(_renderTarget.extent());
			_uniformBuffers.update(index, &ubo);
		}
		_descriptorAllocator.setFrameIndex(index);
		const auto descriptorSet =
			vulkan::DescriptorBuilder{}
				.bindBuffer(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, _uniformBuffers[index])
				.bindImage(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, _textureSampler.handle(), _whiteTexture2D.get<VulkanTexture2D>()->viewHandle(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
				.build(_descriptorAllocator, _pipeline.descriptorSetLayout());
		auto commandBuffer = _context.createCommandBuffer(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		const auto renderPassInfo = _renderTarget.renderPassInfo(index);
		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline.pipeline());
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline.pipelineLayout(), 0, 1, &descriptorSet, 0, nullptr);
		const auto vulkanMesh = static_cast<const VulkanMesh*>(&mesh);
		VkBuffer vertexBuffers[]{ vulkanMesh->vertexBufferHandle() };
		VkDeviceSize offsets[]{ 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, vulkanMesh->indexBufferHandle(), 0, vulkanMesh->indexType());
		vkCmdDrawIndexed(commandBuffer, vulkanMesh->indexCount(), 1, 0, 0, 0);
		vkCmdEndRenderPass(commandBuffer);
		commandBuffer.finish();
		SEIR_VK(vkResetFences(_context._device, 1, &frameFence));
		commandBuffer.submit(_context._graphicsQueue, frameAvailableSemaphore, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, frameRenderedSemaphore, frameFence);
		if (!_renderTarget.presentFrame(_context._presentQueue, index, frameRenderedSemaphore))
			return resetRenderTarget();
		SEIR_VK(vkQueueWaitIdle(_context._presentQueue));
	}

	void VulkanRenderer::resetRenderTarget()
	{
		vkDeviceWaitIdle(_context._device);
		_descriptorAllocator.deallocateAll();
		_pipeline.destroy();
		_renderTarget.destroy(_context._device);
	}

	UniquePtr<Renderer> Renderer::create(const SharedPtr<Window>& window)
	{
		if (auto renderer = makeUnique<VulkanRenderer>(window); renderer->initialize())
			return staticCast<Renderer>(std::move(renderer));
		return {};
	}
}
