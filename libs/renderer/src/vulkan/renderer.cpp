// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "renderer.hpp"

#include <seir_app/window.hpp>
#include <seir_graphics/sizef.hpp>
#include <seir_image/image.hpp>
#include <seir_math/mat.hpp>
#include <seir_renderer/mesh.hpp>
#include "../pass.hpp"
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
		seir::Mat4 _matrix;
	};

	struct PushConstants
	{
		seir::Mat4 _matrix;
	};

	seir::VulkanPipeline createPipeline(const seir::VulkanContext& context, const seir::VulkanRenderTarget& renderTarget, VkShaderModule vertexShader, VkShaderModule fragmentShader, const seir::MeshFormat& meshFormat)
	{
		seir::VulkanPipelineBuilder builder{ renderTarget.extent(), context._maxSampleCount, context._options.sampleShading };
		builder.addDescriptorSetLayout();
		builder.setDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
		builder.addDescriptorSetLayout();
		builder.setDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
		builder.setPushConstantRange(0, sizeof(PushConstants), VK_SHADER_STAGE_VERTEX_BIT);
		builder.setStage(VK_SHADER_STAGE_VERTEX_BIT, vertexShader);
		builder.setStage(VK_SHADER_STAGE_FRAGMENT_BIT, fragmentShader);
		builder.setVertexInput(0, { meshFormat.vertexAttributes.data(), meshFormat.vertexAttributes.size() }, VK_VERTEX_INPUT_RATE_VERTEX);
		builder.setInputAssembly(meshFormat.topology);
		return builder.build(context._device, renderTarget.renderPass());
	}

	class VulkanMesh : public seir::Mesh
	{
	public:
		VulkanMesh(const seir::MeshFormat& format, seir::VulkanBuffer&& vertexBuffer, seir::VulkanBuffer&& indexBuffer, VkIndexType indexType, uint32_t indexCount) noexcept
			: _format{ format }
			, _vertexBuffer{ std::move(vertexBuffer) }
			, _indexBuffer{ std::move(indexBuffer) }
			, _indexType{ indexType }
			, _indexCount{ indexCount }
		{
		}

		constexpr const auto& format() const noexcept { return _format; }
		constexpr auto indexCount() const noexcept { return _indexCount; }
		constexpr auto indexBufferHandle() const noexcept { return _indexBuffer.handle(); }
		constexpr auto indexType() const noexcept { return _indexType; }
		constexpr auto vertexBufferHandle() const noexcept { return _vertexBuffer.handle(); }

	private:
		const seir::MeshFormat _format;
		const seir::VulkanBuffer _vertexBuffer;
		const seir::VulkanBuffer _indexBuffer;
		const VkIndexType _indexType;
		const uint32_t _indexCount;
	};
}

namespace seir
{
	class VulkanShaderSet final : public ShaderSet
	{
	public:
		VulkanShaderSet(seir::VulkanShader&& vertexShader, seir::VulkanShader&& fragmentShader)
			: _vertexShader{ std::move(vertexShader) }, _fragmentShader{ std::move(fragmentShader) } {}
		constexpr auto fragmentShader() const noexcept { return _fragmentShader.handle(); }
		constexpr auto vertexShader() const noexcept { return _vertexShader.handle(); }

	private:
		seir::VulkanShader _vertexShader;
		seir::VulkanShader _fragmentShader;
	};

	class VulkanTexture2D final : public Texture2D
	{
	public:
		VulkanTexture2D(const SizeF& size, VulkanImage&& image) noexcept
			: _size{ size }, _image{ std::move(image) } {}
		SizeF size() const noexcept override { return _size; }
		constexpr auto viewHandle() const noexcept { return _image.viewHandle(); }

	private:
		const SizeF _size;
		VulkanImage _image;
	};

	class VulkanRenderPass final : public RenderPassImpl
	{
	public:
		VulkanRenderPass(VulkanRenderer& renderer, uint32_t frameIndex, const VkDescriptorBufferInfo& uniformBufferInfo, VkCommandBuffer commandBuffer)
			: _renderer{ renderer }
			, _frameIndex{ frameIndex }
			, _uniformBufferInfo{ uniformBufferInfo }
			, _commandBuffer{ commandBuffer }
		{
		}

		void begin2DRendering(const MeshFormat& format)
		{
			selectPipeline(format);
			const auto extent = _renderer._renderTarget.extent();
			setTransformation(seir::Mat4::projection2D(static_cast<float>(extent.width), static_cast<float>(extent.height)));
			processUpdates();
			VkBuffer vertexBuffers[]{ _renderer._2d.vertexBuffer(_frameIndex) };
			VkDeviceSize offsets[]{ 0 };
			vkCmdBindVertexBuffers(_commandBuffer, 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(_commandBuffer, _renderer._2d.indexBuffer(_frameIndex), 0, VK_INDEX_TYPE_UINT16); // TODO: Use actial index type.
		}

		void bindShaders(const SharedPtr<ShaderSet>& shaderSet) override
		{
			assert(shaderSet);
			_shaderSet = staticCast<VulkanShaderSet>(shaderSet);
		}

		void bind2DShaders() override
		{
			_shaderSet = staticCast<VulkanShaderSet>(_renderer._2d.shaders());
		}

		void bindTexture(const SharedPtr<Texture2D>& texture) override
		{
			_texture = texture ? staticCast<VulkanTexture2D>(texture) : _renderer._whiteTexture2D;
			_updateTexture = true;
		}

		void draw2D(uint32_t firstIndex, uint32_t indexCount) override
		{
			processUpdates();
			vkCmdDrawIndexed(_commandBuffer, indexCount, 1, firstIndex, 0, 0);
		}

		void drawMesh(const Mesh& mesh) override
		{
			const auto vulkanMesh = static_cast<const VulkanMesh*>(&mesh);
			selectPipeline(vulkanMesh->format());
			processUpdates();
			VkBuffer vertexBuffers[]{ vulkanMesh->vertexBufferHandle() };
			VkDeviceSize offsets[]{ 0 };
			vkCmdBindVertexBuffers(_commandBuffer, 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(_commandBuffer, vulkanMesh->indexBufferHandle(), 0, vulkanMesh->indexType());
			vkCmdDrawIndexed(_commandBuffer, vulkanMesh->indexCount(), 1, 0, 0, 0);
		}

		void setTransformation(const Mat4& transformation) override
		{
			_pushConstants._matrix = transformation;
			_updatePushConstants = true;
		}

		void update2DBuffers(std::span<const Vertex2D> vertices, std::span<const uint16_t> indices) override
		{
			_renderer._2d.updateBuffers(_renderer._context, _frameIndex, vertices.data(), vertices.size_bytes(), indices.data(), indices.size_bytes());
		}

	private:
		void processUpdates()
		{
			if (_updatePipeline)
			{
				_updatePipeline = false;
				_updateUniformBuffer = true;
				_updateTexture = true;
				_updatePushConstants = true;
				vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline->pipeline());
			}
			if (_updateUniformBuffer)
			{
				_updateUniformBuffer = false;
				const auto descriptorSet =
					vulkan::DescriptorBuilder{}
						.bindBuffer(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, _uniformBufferInfo)
						.build(_renderer._descriptorAllocator, _pipeline->descriptorSetLayout(1));
				vkCmdBindDescriptorSets(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline->pipelineLayout(), 1, 1, &descriptorSet, 0, nullptr);
			}
			if (_updateTexture)
			{
				_updateTexture = false;
				const auto descriptorSet =
					vulkan::DescriptorBuilder{}
						.bindImage(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, _renderer._textureSampler.handle(), _texture->viewHandle(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
						.build(_renderer._descriptorAllocator, _pipeline->descriptorSetLayout(0));
				vkCmdBindDescriptorSets(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline->pipelineLayout(), 0, 1, &descriptorSet, 0, nullptr);
			}
			if (_updatePushConstants)
			{
				_updatePushConstants = false;
				vkCmdPushConstants(_commandBuffer, _pipeline->pipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof _pushConstants, &_pushConstants);
			}
		}

		void selectPipeline(const MeshFormat& meshFormat)
		{
			assert(_shaderSet);
			auto key = static_cast<unsigned>(meshFormat.topology);
			for (size_t i = 0; i < meshFormat.vertexAttributes.size(); ++i)
				key += (static_cast<unsigned>(meshFormat.vertexAttributes[i]) + 1) << (2 * (i + 1));
			auto [i, end] = _renderer._pipelineCache.equal_range(_shaderSet.get());
			while (i != end && i->second.first != key)
				++i;
			if (i == end)
				i = _renderer._pipelineCache.emplace(
					std::piecewise_construct,
					std::forward_as_tuple(_shaderSet.get()),
					std::forward_as_tuple(key, ::createPipeline(_renderer._context, _renderer._renderTarget, _shaderSet->vertexShader(), _shaderSet->fragmentShader(), meshFormat)));
			if (&i->second.second != _pipeline)
			{
				_pipeline = &i->second.second;
				_updatePipeline = true;
			}
		}

	private:
		VulkanRenderer& _renderer;
		const uint32_t _frameIndex;
		const VkDescriptorBufferInfo& _uniformBufferInfo;
		const VkCommandBuffer _commandBuffer;
		SharedPtr<VulkanShaderSet> _shaderSet;
		VulkanPipeline* _pipeline = nullptr;
		bool _updatePipeline = true;
		bool _updateUniformBuffer = true;
		SharedPtr<VulkanTexture2D> _texture = _renderer._whiteTexture2D;
		bool _updateTexture = true;
		PushConstants _pushConstants{ ._matrix = Mat4::identity() };
		bool _updatePushConstants = true;
	};

	VulkanRenderer::VulkanRenderer(const Window& window) noexcept
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
		_pipelineCache.clear();
		_renderTarget.destroy(_context._device);
		_frameSync.destroy(_context._device);
	}

	bool VulkanRenderer::initialize()
	{
		try
		{
			_context.create(_window.descriptor());
			_textureSampler = _context.createSampler2D();
			{
				constexpr uint32_t width = 1;
				constexpr uint32_t height = 1;
				static uint32_t data = 0xffffffff;
				_whiteTexture2D = makeShared<VulkanTexture2D>(SizeF{ width, height },
					_context.createTextureImage2D({ width, height }, VK_FORMAT_B8G8R8A8_SRGB, sizeof(data), &data, width));
			}
			_2d.initialize(*this);
		}
		catch ([[maybe_unused]] const VulkanError& e)
		{
#ifndef NDEBUG
			fmt::print(stderr, "[{}] {}\n", e._function, e._message);
#endif
			return false;
		}
		return true;
	}

	SharedPtr<Mesh> VulkanRenderer::createMesh(const MeshFormat& format, const void* vertexData, size_t vertexCount, const void* indexData, size_t indexCount)
	{
		if (indexCount > std::numeric_limits<uint32_t>::max())
			return {};
		size_t vertexSize = 0;
		for (const auto attribute : format.vertexAttributes)
		{
			switch (attribute)
			{
			case VertexAttribute::f32x2: vertexSize += sizeof(float) * 2; break;
			case VertexAttribute::f32x3: vertexSize += sizeof(float) * 3; break;
			case VertexAttribute::un8x4: vertexSize += sizeof(uint8_t) * 4; break;
			}
		}
		auto vulkanIndexType = VK_INDEX_TYPE_UINT16;
		size_t indexSize = 0;
		switch (format.indexType)
		{
		case MeshIndexType::u16:
			indexSize = sizeof(uint16_t);
			break;
		case MeshIndexType::u32:
			vulkanIndexType = VK_INDEX_TYPE_UINT32;
			indexSize = sizeof(uint32_t);
			break;
		}
		try
		{
			return makeShared<Mesh, VulkanMesh>(format, _context.createDeviceBuffer(vertexData, vertexSize * vertexCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT),
				_context.createDeviceBuffer(indexData, indexSize * indexCount, VK_BUFFER_USAGE_INDEX_BUFFER_BIT), vulkanIndexType, static_cast<uint32_t>(indexCount));
		}
		catch ([[maybe_unused]] const VulkanError& e)
		{
#ifndef NDEBUG
			fmt::print(stderr, "[{}] {}\n", e._function, e._message);
#endif
			return {};
		}
	}

	SharedPtr<ShaderSet> VulkanRenderer::createShaders(std::span<const uint32_t> vertexShader, std::span<const uint32_t> fragmentShader)
	{
		return makeShared<ShaderSet, VulkanShaderSet>(
			_context.createShader(vertexShader.data(), vertexShader.size() * sizeof(uint32_t)),
			_context.createShader(fragmentShader.data(), fragmentShader.size() * sizeof(uint32_t)));
	}

	SharedPtr<Texture2D> VulkanRenderer::createTexture2D(const ImageInfo& info, const void* data)
	{
		if (info.pixelFormat() != PixelFormat::Bgra32)
			return {};
		const auto pixelSize = info.pixelSize();
		const auto stride = info.stride();
		if (stride % pixelSize)
			return {};
		try
		{
			return makeShared<Texture2D, VulkanTexture2D>(SizeF{ static_cast<float>(info.width()), static_cast<float>(info.height()) },
				_context.createTextureImage2D({ info.width(), info.height() }, VK_FORMAT_B8G8R8A8_SRGB, info.frameSize(), data, stride / pixelSize));
		}
		catch ([[maybe_unused]] const VulkanError& e)
		{
#ifndef NDEBUG
			fmt::print(stderr, "[{}] {}\n", e._function, e._message);
#endif
			return {};
		}
	}

	void VulkanRenderer::render(const std::function<Mat4(const Vec2&)>& setup, const std::function<void(RenderPass&)>& callback)
	{
		if (!_renderTarget)
		{
			const auto windowSize = _window.size();
			if (windowSize._width == 0 || windowSize._height == 0)
			{
				sleepFor(1);
				return;
			}
			_renderTarget.create(_context, windowSize);
			const auto frameCount = _renderTarget.frameCount();
			_frameSync.resize(_context._device, frameCount);
			assert(_pipelineCache.empty());
			_uniformBuffers = _context.createUniformBuffers(sizeof(UniformBufferObject), frameCount);
			_descriptorAllocator.reset(_context._device, frameCount, 1'000,
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
			_2d.resize(frameCount);
		}
		const auto [frameAvailableSemaphore, frameRenderedSemaphore, frameFence] = _frameSync.switchFrame(_context._device);
		uint32_t index = 0;
		if (!_renderTarget.acquireFrame(_context._device, frameAvailableSemaphore, frameFence, index))
			return resetRenderTarget();
		{
			const auto viewportSize = _renderTarget.extent();
			const UniformBufferObject ubo{
				._matrix = setup({ static_cast<float>(viewportSize.width), static_cast<float>(viewportSize.height) }),
			};
			_uniformBuffers.update(index, &ubo);
		}
		_descriptorAllocator.setFrameIndex(index);
		auto commandBuffer = _context.createCommandBuffer(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		const auto renderPassInfo = _renderTarget.renderPassInfo(index);
		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		{
			VulkanRenderPass renderPass{ *this, index, _uniformBuffers[index], commandBuffer };
			callback(renderPass);
		}
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
		_pipelineCache.clear(); // TODO: Reset pipelines without memory deallocation.
		_renderTarget.destroy(_context._device);
	}

	UniquePtr<Renderer> Renderer::create(const Window& window)
	{
		if (auto renderer = makeUnique<VulkanRenderer>(window); renderer->initialize())
			return staticCast<Renderer>(std::move(renderer));
		return {};
	}
}
