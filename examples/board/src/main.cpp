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
#include <seir_io/inlet.hpp>
#include <seir_math/euler.hpp>
#include <seir_math/mat.hpp>
#include <seir_math/plane.hpp>
#include <seir_math/ray.hpp>
#include <seir_model/mesh_data.hpp>
#include <seir_renderer/2d.hpp>
#include <seir_renderer/renderer.hpp>
#include <seir_u8main/u8main.hpp>

#include <cmath>
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

	const uint32_t kVertexShaderV3N3[]{
#if SEIR_RENDERER_VULKAN
#	include "vertex_v3n3.glsl.spirv.inc"
#else
		0
#endif
	};

	const uint32_t kVertexShaderV3N3T2[]{
#if SEIR_RENDERER_VULKAN
#	include "vertex_v3n3t2.glsl.spirv.inc"
#else
		0
#endif
	};

	const uint32_t kFragmentShaderV3N3[]{
#if SEIR_RENDERER_VULKAN
#	include "fragment_v3n3.glsl.spirv.inc"
#else
		0
#endif
	};

	const uint32_t kFragmentShaderV3N3T2[]{
#if SEIR_RENDERER_VULKAN
#	include "fragment_v3n3t2.glsl.spirv.inc"
#else
		0
#endif
	};

	class Example
	{
	public:
		std::optional<seir::Vec2> cursor() const noexcept
		{
			return _cursor;
		}

		void presentGui(seir::GuiFrame&& frame)
		{
			seir::GuiLayout layout{ frame };
			layout.setItemSize({ 0, 16 });
			frame.setLabelStyle({ seir::Rgba32::red(), 1 });
			layout.fromTopLeft(seir::GuiLayout::Axis::Y, 4);
			frame.addLabel(_debugText, seir::GuiAlignment::Left);
			layout.fromTopRight(seir::GuiLayout::Axis::Y, 4);
			frame.addLabel(_fps, seir::GuiAlignment::Right);
			layout.fromTopLeft(seir::GuiLayout::Axis::Y, 0);
			_cursor = frame.addHoverArea(frame.size());
			if (frame.takeKeyPress(seir::Key::Escape))
				frame.close();
		}

		void setDebugText(const std::string& text)
		{
			_debugText = text;
		}

		void setFps(float fps)
		{
			_fps.clear();
			std::format_to(std::back_inserter(_fps), "{:.1f} fps", fps);
		}

	private:
		std::optional<seir::Vec2> _cursor;
		std::string _fps;
		std::string _debugText;
	};

	seir::Ray3D pixelRay(const seir::Vec2& point, const seir::Vec2& viewportSize, const seir::Mat4& viewMatrix, const seir::Vec3& cameraPosition)
	{
		// Point coordinates should be in [0, D) range (where D is width or height).
		// We shift coordinates to the center of the pixel (by adding 0.5),
		// then normalize them from [0, D] to [-1, 1].
		const auto x = (2 * point.x + 1) / viewportSize.x - 1;
		const auto y = (2 * point.y + 1) / viewportSize.y - 1;
		const auto m = inverse(viewMatrix);
		return seir::Ray3D::fromPoints(cameraPosition, m * seir::Vec3{ x, y, 1 });
	}
}

int u8main(int, char**)
{
	const seir::Plane kBoardPlane{ { 0, 0, 1 }, { 0, 0, 0 } };

	seir::App app;
	seir::Window window{ app, "Board" };
	seir::Renderer renderer{ window };
	const auto boardTexture = ::makeBgra32(renderer, 128, 128, [](size_t x, size_t y) {
		return ((x ^ y) & 1) ? seir::Rgba32::grayscale(0xdd) : seir::Rgba32::black();
	});
	const auto boardMesh = renderer.createMesh(seir::MeshData::load(seir::fromFile(LOCAL_DATA_DIR "board.obj")));
	const auto boardShaders = renderer.createShaders(kVertexShaderV3N3T2, kFragmentShaderV3N3T2);
	const auto cubeMesh = renderer.createMesh(seir::MeshData::load(seir::fromFile(LOCAL_DATA_DIR "cube.obj")));
	const auto cubeShaders = renderer.createShaders(kVertexShaderV3N3, kFragmentShaderV3N3);
	seir::Renderer2D renderer2d;
	seir::GuiContext gui{ window, seir::Font::load(renderer, seir::fromFile(SEIR_DATA_DIR "fonts/SourceCodePro-Regular.ttf"), 16) };
	Example example;
	for (seir::VariableRate clock; app.processEvents(gui.eventCallbacks());)
	{
		example.presentGui({ gui, renderer2d });
		renderer.render([&](seir::RenderPass& pass) {
			const auto viewportSize = pass.size();
			const seir::Vec3 cameraPosition{ 0, -8.5, 16 };
			const auto viewMatrix = seir::Mat4::projection3D(viewportSize.x / viewportSize.y, 35, .5) * seir::Mat4::camera(cameraPosition, { 0, -60, 0 });
			pass.updateUniformBuffer(viewMatrix);
			pass.bindShaders(boardShaders);
			pass.bindTexture(boardTexture);
			pass.bindUniformBuffer(true);
			pass.setTransformation(seir::Mat4::identity());
			pass.drawMesh(*boardMesh);
			if (const auto cursor = example.cursor())
			{
				const auto cursorRay = ::pixelRay(*cursor, viewportSize, viewMatrix, cameraPosition);
				if (const auto boardPoint = cursorRay.intersection(kBoardPlane);
					boardPoint && std::abs(boardPoint->x) <= 64 && std::abs(boardPoint->y) <= 64)
				{
					const auto boardX = std::floor(boardPoint->x);
					const auto boardY = std::floor(boardPoint->y);
					example.setDebugText(std::format("({:+},{:+})", boardX, boardY));
					pass.bindShaders(cubeShaders);
					pass.setTransformation(seir::Mat4::translation({ boardX + .5f, boardY + .5f, .5 }));
					pass.drawMesh(*cubeMesh);
				}
			}
			renderer2d.draw(pass);
		});
		if (const auto period = clock.advance())
			example.setFps(period->_averageFrameRate);
	}
	return 0;
}
