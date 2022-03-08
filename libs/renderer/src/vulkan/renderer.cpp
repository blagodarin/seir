// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "renderer.hpp"

#include <seir_app/window.hpp>
#include <seir_math/euler.hpp>
#include <seir_math/mat.hpp>
#include "error.hpp"
#include "utils.hpp"

namespace
{
	struct UniformBufferObject
	{
		seir::Mat4 _model;
		seir::Mat4 _view;
		seir::Mat4 _projection;
	};

	struct Vertex
	{
		seir::Vec3 position;
		seir::Vec3 color;
		seir::Vec2 texCoord;
	};

	const uint32_t kVertexShader[]{
#include "vertex_shader.glsl.spirv.inc"
	};

	const uint32_t kFragmentShader[]{
#include "fragment_shader.glsl.spirv.inc"
	};

	static constexpr std::array<uint8_t, 16> kTextureData{
		0x99, 0xbb, 0xbb, 0xff,
		0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff
	};

	constexpr std::array kVertexData{
		Vertex{ .position{ -1, -1, .5 }, .color{ 1, 0, 0 }, .texCoord{ 0, 0 } },
		Vertex{ .position{ 1, -1, .5 }, .color{ 1, 1, 1 }, .texCoord{ 1, 0 } },
		Vertex{ .position{ -1, 1, .5 }, .color{ 0, 1, 0 }, .texCoord{ 0, 1 } },
		Vertex{ .position{ 1, 1, .5 }, .color{ 0, 0, 1 }, .texCoord{ 1, 1 } },

		Vertex{ .position{ -1, -1, 0 }, .color{ 1, 1, 0 }, .texCoord{ 0, 0 } },
		Vertex{ .position{ 1, -1, 0 }, .color{ 0, 1, 1 }, .texCoord{ 1, 0 } },
		Vertex{ .position{ -1, 1, 0 }, .color{ 1, 0, 1 }, .texCoord{ 0, 1 } },
		Vertex{ .position{ 1, 1, 0 }, .color{ 0, 0, 0 }, .texCoord{ 1, 1 } },
	};

	constexpr std::array<uint16_t, 10> kIndexData{
		0, 1, 2, 3,
		0xffff,
		4, 5, 6, 7
	};

	seir::VulkanPipeline createPipeline(const seir::VulkanContext& context, const seir::VulkanRenderTarget& renderTarget, VkShaderModule vertexShader, VkShaderModule fragmentShader)
	{
		seir::VulkanPipelineBuilder builder{ renderTarget._swapchainExtent, context._maxSampleCount, context._options.sampleShading };
		builder.setDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
		builder.setDescriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
		builder.setStage(VK_SHADER_STAGE_VERTEX_BIT, vertexShader);
		builder.setStage(VK_SHADER_STAGE_FRAGMENT_BIT, fragmentShader);
		builder.setVertexInput(0, { seir::VertexAttribute::f32x3, seir::VertexAttribute::f32x3, seir::VertexAttribute::f32x2 });
		builder.setInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, true);
		return builder.build(context._device, renderTarget._renderPass, static_cast<uint32_t>(renderTarget._swapchainImages.size()));
	}

	UniformBufferObject makeUniformBuffer(const VkExtent2D& screenSize)
	{
		return {
			._model = seir::Mat4::rotation(10 * seir::clockTime(), { 0, 0, 1 }),
			._view = seir::Mat4::camera({ 0, -3, 3 }, { 0, -45, 0 }),
			._projection = seir::Mat4::projection3D(static_cast<float>(screenSize.width) / static_cast<float>(screenSize.height), 45, 1),
		};
	}
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
		_commandBuffers.clear();
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
			_textureImage = _context.createTextureImage2D({ 1, 2 }, VK_FORMAT_B8G8R8A8_SRGB, kTextureData.size(), kTextureData.data(), 2);
			_textureSampler = _context.createSampler2D();
			_vertexBuffer = _context.createDeviceBuffer(kVertexData.data(), kVertexData.size() * sizeof(Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
			_indexBuffer = _context.createDeviceBuffer(kIndexData.data(), kIndexData.size() * sizeof(uint16_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
			_frameSync.create(_context._device);
			return true;
		}
		catch ([[maybe_unused]] const seir::VulkanError& e)
		{
#ifndef NDEBUG
			fmt::print(stderr, "[{}] {}\n", e._function, e._message);
#endif
			return false;
		}
	}

	void VulkanRenderer::draw()
	{
		if (!_renderTarget._swapchain)
		{
			const auto windowSize = _window->size();
			if (windowSize._width == 0 || windowSize._height == 0)
			{
				sleepFor(1);
				return;
			}
			_renderTarget.create(_context, windowSize);
			_pipeline = ::createPipeline(_context, _renderTarget, _vertexShader.handle(), _fragmentShader.handle());
			_uniformBuffers = _context.createUniformBuffers(sizeof(UniformBufferObject), _renderTarget._swapchainImages.size());
			for (size_t i = 0; i < _renderTarget._swapchainImages.size(); ++i)
			{
				const auto descriptorSet =
					VulkanDescriptorBuilder{}
						.bindBuffer(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, _uniformBuffers[i])
						.bindImage(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, _textureSampler.handle(), _textureImage.viewHandle(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
						.build(_context._device, _pipeline.descriptorPool(), _pipeline.descriptorSetLayout());
				auto commandBuffer = _context.createCommandBuffer(0);
				const auto renderPassInfo = _renderTarget.renderPassInfo(i);
				vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline.pipeline());
				VkBuffer vertexBuffers[]{ _vertexBuffer.handle() };
				VkDeviceSize offsets[]{ 0 };
				vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
				vkCmdBindIndexBuffer(commandBuffer, _indexBuffer.handle(), 0, VK_INDEX_TYPE_UINT16);
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline.pipelineLayout(), 0, 1, &descriptorSet, 0, nullptr);
				vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(kIndexData.size()), 1, 0, 0, 0);
				vkCmdEndRenderPass(commandBuffer);
				commandBuffer.finish();
				_commandBuffers.emplace_back(std::move(commandBuffer));
			}
		}
		const auto [imageAvailableSemaphore, renderFinishedSemaphore, fence] = _frameSync.switchFrame(_context._device);
		uint32_t index = 0;
		if (const auto status = vkAcquireNextImageKHR(_context._device, _renderTarget._swapchain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &index); status == VK_ERROR_OUT_OF_DATE_KHR)
		{
			vkDeviceWaitIdle(_context._device);
			_commandBuffers.clear();
			_pipeline.destroy();
			_renderTarget.destroy(_context._device);
			return;
		}
		else if (status != VK_SUCCESS && status != VK_SUBOPTIMAL_KHR)
			SEIR_VK_THROW("vkAcquireNextImageKHR", status);
		if (_renderTarget._swapchainImageFences[index])
			SEIR_VK(vkWaitForFences(_context._device, 1, &_renderTarget._swapchainImageFences[index], VK_TRUE, UINT64_MAX));
		_renderTarget._swapchainImageFences[index] = fence;
		{
			const auto ubo = ::makeUniformBuffer(_renderTarget._swapchainExtent);
			_uniformBuffers.update(index, &ubo);
		}
		const VkSemaphore waitSemaphores[]{ imageAvailableSemaphore };
		const VkPipelineStageFlags waitStages[]{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		const VkSemaphore signalSemaphores[]{ renderFinishedSemaphore };
		const auto commandBuffer = VkCommandBuffer{ _commandBuffers[index] };
		const VkSubmitInfo submitInfo{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = waitSemaphores,
			.pWaitDstStageMask = waitStages,
			.commandBufferCount = 1,
			.pCommandBuffers = &commandBuffer,
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = signalSemaphores,
		};
		SEIR_VK(vkResetFences(_context._device, 1, &fence));
		SEIR_VK(vkQueueSubmit(_context._graphicsQueue, 1, &submitInfo, fence));
		const VkSwapchainKHR swapchains[]{ _renderTarget._swapchain };
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
			_commandBuffers.clear();
			_pipeline.destroy();
			_renderTarget.destroy(_context._device);
			return;
		}
		else if (status != VK_SUCCESS)
			SEIR_VK_THROW("vkQueuePresentKHR", status);
		SEIR_VK(vkQueueWaitIdle(_context._presentQueue));
	}

	UniquePtr<Renderer> Renderer::create(const SharedPtr<Window>& window)
	{
		if (auto renderer = makeUnique<VulkanRenderer>(window); renderer->initialize())
			return staticCast<Renderer>(std::move(renderer));
		return {};
	}
}
