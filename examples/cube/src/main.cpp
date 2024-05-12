// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_app/app.hpp>
#include <seir_app/events.hpp>
#include <seir_app/window.hpp>
#include <seir_base/clock.hpp>
#include <seir_data/blob.hpp>
#include <seir_graphics/color.hpp>
#include <seir_graphics/rectf.hpp>
#include <seir_gui/font.hpp> // TODO: Get rid of GUI in this example.
#include <seir_image/image.hpp>
#include <seir_math/euler.hpp>
#include <seir_math/mat.hpp>
#include <seir_renderer/mesh.hpp>
#include <seir_renderer/2d.hpp>
#include <seir_renderer/renderer.hpp>
#include <seir_u8main/u8main.hpp>

#include <array>

#include <fmt/core.h>

namespace
{
	// TODO: Load cube model from file.

	constexpr std::array<uint8_t, 64> kTextureData{
		0xff, 0xff, 0xff, 0xff, 0xcc, 0xcc, 0xcc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xcc, 0xcc, 0xcc, 0xff,
		0xcc, 0xcc, 0xcc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xcc, 0xcc, 0xcc, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xcc, 0xcc, 0xcc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xcc, 0xcc, 0xcc, 0xff,
		0xcc, 0xcc, 0xcc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xcc, 0xcc, 0xcc, 0xff, 0xff, 0xff, 0xff, 0xff
	};

	const seir::MeshFormat kMeshFormat{
		.vertexAttributes{
			seir::VertexAttribute::f32x3,
			seir::VertexAttribute::f32x3,
			seir::VertexAttribute::f32x3,
			seir::VertexAttribute::f32x2,
		},
		.topology = seir::MeshTopology::TriangleStrip,
		.indexType = seir::MeshIndexType::u16,
	};

	struct Vertex
	{
		seir::Vec3 position;
		seir::Vec3 normal;
		seir::Vec3 color;
		seir::Vec2 texCoord;
	};

	constexpr std::array kVertexData{
		// Top.
		Vertex{ .position{ -1, -1, 1 }, .normal{ 0, 0, 1 }, .color{ 1, 0, 0 }, .texCoord{ 0, 1 } },
		Vertex{ .position{ 1, -1, 1 }, .normal{ 0, 0, 1 }, .color{ 1, 0, 0 }, .texCoord{ 1, 1 } },
		Vertex{ .position{ -1, 1, 1 }, .normal{ 0, 0, 1 }, .color{ 1, 0, 0 }, .texCoord{ 0, 0 } },
		Vertex{ .position{ 1, 1, 1 }, .normal{ 0, 0, 1 }, .color{ 1, 0, 0 }, .texCoord{ 1, 0 } },

		// Front.
		Vertex{ .position{ -1, -1, -1 }, .normal{ 0, -1, 0 }, .color{ 1, 1, 0 }, .texCoord{ 0, 0 } },
		Vertex{ .position{ 1, -1, -1 }, .normal{ 0, -1, 0 }, .color{ 1, 1, 0 }, .texCoord{ 1, 0 } },
		Vertex{ .position{ -1, -1, 1 }, .normal{ 0, -1, 0 }, .color{ 1, 1, 0 }, .texCoord{ 0, 1 } },
		Vertex{ .position{ 1, -1, 1 }, .normal{ 0, -1, 0 }, .color{ 1, 1, 0 }, .texCoord{ 1, 1 } },

		// Left.
		Vertex{ .position{ -1, 1, -1 }, .normal{ -1, 0, 0 }, .color{ 1, 0, 1 }, .texCoord{ 1, 0 } },
		Vertex{ .position{ -1, -1, -1 }, .normal{ -1, 0, 0 }, .color{ 1, 0, 1 }, .texCoord{ 0, 0 } },
		Vertex{ .position{ -1, 1, 1 }, .normal{ -1, 0, 0 }, .color{ 1, 0, 1 }, .texCoord{ 1, 1 } },
		Vertex{ .position{ -1, -1, 1 }, .normal{ -1, 0, 0 }, .color{ 1, 0, 1 }, .texCoord{ 0, 1 } },

		// Right.
		Vertex{ .position{ 1, -1, -1 }, .normal{ 1, 0, 0 }, .color{ 0, 1, 0 }, .texCoord{ 1, 0 } },
		Vertex{ .position{ 1, 1, -1 }, .normal{ 1, 0, 0 }, .color{ 0, 1, 0 }, .texCoord{ 0, 0 } },
		Vertex{ .position{ 1, -1, 1 }, .normal{ 1, 0, 0 }, .color{ 0, 1, 0 }, .texCoord{ 1, 1 } },
		Vertex{ .position{ 1, 1, 1 }, .normal{ 1, 0, 0 }, .color{ 0, 1, 0 }, .texCoord{ 0, 1 } },

		// Back.
		Vertex{ .position{ 1, 1, -1 }, .normal{ 0, 1, 0 }, .color{ 0, 1, 1 }, .texCoord{ 0, 0 } },
		Vertex{ .position{ -1, 1, -1 }, .normal{ 0, 1, 0 }, .color{ 0, 1, 1 }, .texCoord{ 1, 0 } },
		Vertex{ .position{ 1, 1, 1 }, .normal{ 0, 1, 0 }, .color{ 0, 1, 1 }, .texCoord{ 0, 1 } },
		Vertex{ .position{ -1, 1, 1 }, .normal{ 0, 1, 0 }, .color{ 0, 1, 1 }, .texCoord{ 1, 1 } },

		// Bottom.
		Vertex{ .position{ -1, 1, -1 }, .normal{ 0, 0, -1 }, .color{ 0, 0, 1 }, .texCoord{ 0, 1 } },
		Vertex{ .position{ 1, 1, -1 }, .normal{ 0, 0, -1 }, .color{ 0, 0, 1 }, .texCoord{ 1, 1 } },
		Vertex{ .position{ -1, -1, -1 }, .normal{ 0, 0, -1 }, .color{ 0, 0, 1 }, .texCoord{ 0, 0 } },
		Vertex{ .position{ 1, -1, -1 }, .normal{ 0, 0, -1 }, .color{ 0, 0, 1 }, .texCoord{ 1, 0 } },
	};

	constexpr std::array<uint16_t, 29> kIndexData{
		0, 1, 2, 3,
		0xffff,
		4, 5, 6, 7,
		0xffff,
		8, 9, 10, 11,
		0xffff,
		12, 13, 14, 15,
		0xffff,
		16, 17, 18, 19,
		0xffff,
		20, 21, 22, 23
	};

	const uint32_t kVertexShader[]
	{
#if SEIR_RENDERER_VULKAN
#	include "vertex_shader.glsl.spirv.inc"
#else
		0
#endif
	};

	const uint32_t kFragmentShader[]
	{
#if SEIR_RENDERER_VULKAN
#	include "fragment_shader.glsl.spirv.inc"
#else
		0
#endif
	};

	class State : public seir::EventCallbacks
	{
	public:
		seir::Mat4 cameraMatrix() const noexcept
		{
			return seir::Mat4::camera(_cameraPosition, { 0, 0, 0 });
		}

	public:
		void onKeyEvent(seir::Window& window, const seir::KeyEvent& event) override
		{
			if (!event._pressed || event._repeated)
				return;
			if (event._key == seir::Key::Escape)
				window.close();
			else if (event._key == seir::Key::Down)
				_cameraPosition.z -= .25f;
			else if (event._key == seir::Key::Up)
				_cameraPosition.z += .25f;
		}

	private:
		seir::Vec3 _cameraPosition{ 0, -5, 0 };
	};
}

int u8main(int, char**)
{
	// TODO: Make less verbose.
	// Posible options include:
	// - making create() return SharedPtr;
	// - removing SharedPtr and passing App and Window by reference.
	const auto app = seir::SharedPtr{ seir::App::create() };
	const auto window = seir::SharedPtr{ seir::Window::create(app, "Cube") };
	const auto renderer = seir::Renderer::create(window);
	seir::Renderer2D renderer2d;
	const auto texture = renderer->createTexture2D({ 4, 4, seir::PixelFormat::Bgra32 }, kTextureData.data());
	const auto mesh = renderer->createMesh(kMeshFormat, kVertexData.data(), kVertexData.size(), kIndexData.data(), kIndexData.size());
	const auto shaders = renderer->createShaders(kVertexShader, kFragmentShader);
	const auto font = seir::Font::load(seir::Blob::from(SEIR_DATA_DIR "source_sans_pro.ttf"), 20, *renderer);
	window->show();
	seir::VariableRate clock;
	std::string fps1;
	std::string fps2;
	for (State state; app->processEvents(state);)
	{
		const auto time = clock.time();
		renderer->render(
			[&state](const seir::Vec2& viewportSize) {
				return seir::Mat4::projection3D(viewportSize.x / viewportSize.y, 45, 1) * state.cameraMatrix();
			},
			[&](seir::RenderPass& pass) {
				pass.bindShaders(shaders);
				pass.bindTexture(texture);
				pass.setTransformation(seir::Mat4::rotation(29 * time, { 0, 0, 1 }) * seir::Mat4::rotation(19 * time, { 1, 0, 0 }));
				pass.drawMesh(*mesh);
				font->renderLine(renderer2d, { { 5, 5 }, seir::SizeF{ 200, 20 } }, fps1);
				font->renderLine(renderer2d, { { 5, 25 }, seir::SizeF{ 200, 20 } }, fps2);
				renderer2d.draw(pass);
			});
		if (const auto period = clock.advance())
		{
			fps1.clear();
			fmt::format_to(std::back_inserter(fps1), "{:.1f} fps", period->_averageFrameRate);
			fps2.clear();
			fmt::format_to(std::back_inserter(fps2), "{:.1f} < {} ms/f", 1000 / period->_averageFrameRate, period->_maxFrameDuration);
		}
	}
	return 0;
}
