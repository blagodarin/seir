// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_app/app.hpp>
#include <seir_app/window.hpp>
#include <seir_base/clock.hpp>
#include <seir_graphics/color.hpp>
#include <seir_gui/context.hpp>
#include <seir_gui/font.hpp>
#include <seir_gui/frame.hpp>
#include <seir_gui/layout.hpp>
#include <seir_gui/style.hpp>
#include <seir_image/image.hpp>
#include <seir_io/blob.hpp>
#include <seir_math/euler.hpp>
#include <seir_math/mat.hpp>
#include <seir_model/mesh_data.hpp>
#include <seir_renderer/2d.hpp>
#include <seir_renderer/renderer.hpp>
#include <seir_u8main/u8main.hpp>

#include <format>

namespace
{
	template <typename Callback>
	auto makeBgra32(seir::Renderer& renderer, uint32_t width, uint32_t height, Callback&& callback)
	{
		seir::ImageInfo imageInfo{ width, height, seir::PixelFormat::Bgra32 };
		seir::Buffer imageData{ imageInfo.frameSize() };
		for (size_t y = 0; y < height; ++y)
		{
			for (size_t x = 0; x < width; ++x)
			{
				const auto pixel = reinterpret_cast<uint8_t*>(imageData.data() + y * imageInfo.stride() + x * 4);
				const auto color = callback(x, y);
				pixel[0] = color._b;
				pixel[1] = color._g;
				pixel[2] = color._r;
				pixel[3] = color._a;
			}
		}
		return renderer.createTexture2D(imageInfo, imageData.data());
	}

	const uint32_t kBoardVertexShader[]{
#if SEIR_RENDERER_VULKAN
#	include "board_vs.glsl.spirv.inc"
#else
		0
#endif
	};

	const uint32_t kBoardFragmentShader[]{
#if SEIR_RENDERER_VULKAN
#	include "board_fs.glsl.spirv.inc"
#else
		0
#endif
	};

	class Example
	{
	public:
		void presentGui(seir::GuiFrame&& frame)
		{
			seir::GuiLayout layout{ frame };
			layout.fromTopRight(seir::GuiLayout::Axis::Y, 4);
			layout.setItemSize({ 0, 16 });
			frame.setLabelStyle({ seir::Rgba32::red(), 1 });
			frame.addLabel(_fps, seir::GuiAlignment::Right);
			if (frame.takeKeyPress(seir::Key::Escape))
				frame.close();
		}

		void setFps(float fps)
		{
			_fps.clear();
			std::format_to(std::back_inserter(_fps), "{:.1f} fps", fps);
		}

	private:
		std::string _fps;
	};
}

int u8main(int, char**)
{
	seir::App app;
	seir::Window window{ app, "Board" };
	seir::Renderer renderer{ window };
	const auto boardTexture = ::makeBgra32(renderer, 128, 128, [](size_t x, size_t y) {
		return ((x ^ y) & 1) ? seir::Rgba32::grayscale(0xdd) : seir::Rgba32::black();
	});
	const auto boardMeshData = seir::MeshData::create(seir::load(LOCAL_DATA_DIR "board.obj"));
	if (!boardMeshData)
		return 1;
	const auto boardMesh = renderer.createMesh(boardMeshData->format(), boardMeshData->vertexData(), boardMeshData->vertexCount(), boardMeshData->indexData(), boardMeshData->indexCount());
	const auto boardShaders = renderer.createShaders(kBoardVertexShader, kBoardFragmentShader);
	seir::Renderer2D renderer2d;
	seir::GuiContext gui{ window, seir::Font::create(renderer, seir::load(SEIR_DATA_DIR "fonts/SourceCodePro-Regular.ttf"), 16) };
	Example example;
	for (seir::VariableRate clock; app.processEvents(gui.eventCallbacks());)
	{
		example.presentGui({ gui, renderer2d });
		renderer.render([&](seir::RenderPass& pass) {
			const auto viewportSize = pass.size();
			pass.updateUniformBuffer(seir::Mat4::projection3D(viewportSize.x / viewportSize.y, 35, .5) * seir::Mat4::camera({ 0, -8.5, 16 }, { 0, -60, 0 }));
			pass.bindShaders(boardShaders);
			pass.bindTexture(boardTexture);
			pass.bindUniformBuffer(true);
			pass.setTransformation(seir::Mat4::identity());
			pass.drawMesh(*boardMesh);
			renderer2d.draw(pass);
		});
		if (const auto period = clock.advance())
			example.setFps(period->_averageFrameRate);
	}
	return 0;
}
