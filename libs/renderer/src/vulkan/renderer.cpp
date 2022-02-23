// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "renderer.hpp"

#include <seir_app/window.hpp>
#include "error.hpp"
#include "utils.hpp"

namespace
{
	const uint32_t kVertexShader[]{
#include "vertex_shader.glsl.spirv.inc"
	};

	const uint32_t kFragmentShader[]{
#include "fragment_shader.glsl.spirv.inc"
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
		return builder.build(context._device, renderTarget._renderPass);
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
		_swapchain.destroy(_context._device, _context._commandPool);
		_pipeline.destroy();
		_renderTarget.destroy(_context._device);
		_frameSync.destroy(_context._device);
	}

	bool VulkanRenderer::initialize()
	{
		try
		{
			_context.create(_window->descriptor());
			_vertexShader.create(_context._device, kVertexShader, sizeof kVertexShader);
			_fragmentShader.create(_context._device, kFragmentShader, sizeof kFragmentShader);
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
			_swapchain.create(_context, _renderTarget, _pipeline);
		}
		const auto [imageAvailableSemaphore, renderFinishedSemaphore, fence] = _frameSync.switchFrame(_context._device);
		uint32_t index = 0;
		if (const auto status = vkAcquireNextImageKHR(_context._device, _renderTarget._swapchain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &index); status == VK_ERROR_OUT_OF_DATE_KHR)
		{
			vkDeviceWaitIdle(_context._device);
			_swapchain.destroy(_context._device, _context._commandPool);
			_pipeline.destroy();
			_renderTarget.destroy(_context._device);
			return;
		}
		else if (status != VK_SUCCESS && status != VK_SUBOPTIMAL_KHR)
			SEIR_VK_THROW("vkAcquireNextImageKHR", status);
		if (_renderTarget._swapchainImageFences[index])
			SEIR_VK(vkWaitForFences(_context._device, 1, &_renderTarget._swapchainImageFences[index], VK_TRUE, UINT64_MAX));
		_renderTarget._swapchainImageFences[index] = fence;
		_swapchain.updateUniformBuffer(_context._device, index, _renderTarget._swapchainExtent);
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
			_swapchain.destroy(_context._device, _context._commandPool);
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
