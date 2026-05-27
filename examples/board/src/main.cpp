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
#include <seir_model/mesh_format.hpp>
#include <seir_renderer/2d.hpp>
#include <seir_renderer/renderer.hpp>
#include <seir_u8main/u8main.hpp>

#include <array>
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

	struct Vertex
	{
		seir::Vec3 position;
		seir::Vec3 normal;
		seir::Vec2 texCoord;
	};

	constexpr std::array kBoardVertices{
		Vertex{ .position{ -64, -64, 0 }, .normal{ 0, 0, 1 }, .texCoord{ 0, 0 } },
		Vertex{ .position{ 64, -64, 0 }, .normal{ 0, 0, 1 }, .texCoord{ 1, 0 } },
		Vertex{ .position{ -64, 64, 0 }, .normal{ 0, 0, 1 }, .texCoord{ 0, 1 } },
		Vertex{ .position{ 64, 64, 0 }, .normal{ 0, 0, 1 }, .texCoord{ 1, 1 } },
	};

	constexpr std::array<uint16_t, 4> kBoardIndices{
		0,
		1,
		2,
		3,
	};

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
	// MeshFormat can't be constexpr so it requires
	// a global destructor which we don't want to allow.
	const seir::MeshFormat kBoardMeshFormat{
		.vertexAttributes{
			seir::VertexAttribute::f32x3,
			seir::VertexAttribute::f32x3,
			seir::VertexAttribute::f32x2,
		},
		.topology = seir::MeshTopology::TriangleStrip,
		.indexType = seir::MeshIndexType::u16,
	};

	seir::App app;
	seir::Window window{ app, "Board" };
	seir::Renderer renderer{ window };
	const auto boardTexture = ::makeBgra32(renderer, 128, 128, [](size_t x, size_t y) {
		return ((x ^ y) & 1) ? seir::Rgba32::grayscale(0xdd) : seir::Rgba32::black();
	});
	const auto boardMesh = renderer.createMesh(kBoardMeshFormat, kBoardVertices.data(), kBoardVertices.size(), kBoardIndices.data(), kBoardIndices.size());
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
