// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_app/app.hpp>
#include <seir_app/window.hpp>
#include <seir_base/clock.hpp>
#include <seir_graphics/color.hpp>
#include <seir_graphics/quadf.hpp>
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

	class Camera3D
	{
	public:
		Camera3D(const seir::SizeF& viewportSize, const seir::Vec3& cameraPosition) noexcept
			: _viewportSize{ viewportSize._width, viewportSize._height }
			, _cameraPosition{ cameraPosition }
			, _viewMatrix{ seir::Mat4::projection3D(_viewportSize.x / _viewportSize.y, 35, .5) * seir::Mat4::camera(_cameraPosition, { 0, -60, 0 }) }
		{
		}

		seir::Ray3D pixelRay(const seir::Vec2& point) const noexcept
		{
			// Point coordinates should be in [0, D) range (where D is width or height).
			// We shift coordinates to the center of the pixel (by adding 0.5),
			// then normalize them from [0, D] to [-1, 1].
			const auto x = (2 * point.x + 1) / _viewportSize.x - 1;
			const auto y = (2 * point.y + 1) / _viewportSize.y - 1;
			return seir::Ray3D::fromPoints(_cameraPosition, _inverseViewMatrix * seir::Vec3{ x, y, 1 });
		}

		std::optional<seir::Vec3> pixelRayIntersection(const std::optional<seir::Vec2>& pixel, const seir::Plane& plane) const noexcept
		{
			if (pixel.has_value())
				return pixelRay(*pixel).intersection(plane);
			return {};
		}

		const seir::Mat4& viewMatrix() const noexcept { return _viewMatrix; }

		seir::QuadF viewportQuad(const seir::Plane& plane, const seir::Vec3& center) const noexcept
		{
			const auto planeIntersection = [this, &plane](float x, float y) {
				return seir::Ray3D::fromPoints(_cameraPosition, _inverseViewMatrix * seir::Vec3{ x, y, 1 }).intersection(plane);
			};

			if (const auto topLeft = planeIntersection(-1, -1)) [[likely]]
				if (const auto topRight = planeIntersection(1, -1)) [[likely]]
					if (const auto bottomLeft = planeIntersection(-1, 1)) [[likely]]
						if (const auto bottomRight = planeIntersection(1, 1)) [[likely]]
						{
							const auto origin = center - plane.distanceTo(center) * plane.normal();

							const auto up = normalize(*topLeft - *bottomLeft + *topRight - *bottomRight);
							const auto right = crossProduct(up, plane.normal());

							const seir::Mat4 planeMatrix{
								right.x, up.x, plane.normal().x, origin.x,
								right.y, up.y, plane.normal().y, origin.y,
								right.z, up.z, plane.normal().z, origin.z,
								0, 0, 0, 1
							};

							const auto inversePlaneMatrix = inverse(planeMatrix);

							const auto topLeft2D = inversePlaneMatrix * *topLeft;
							const auto topRight2D = inversePlaneMatrix * *topRight;
							const auto bottomLeft2D = inversePlaneMatrix * *bottomLeft;
							const auto bottomRight2D = inversePlaneMatrix * *bottomRight;

							return {
								{ topLeft2D.x, topLeft2D.y },
								{ topRight2D.x, topRight2D.y },
								{ bottomLeft2D.x, bottomLeft2D.y },
								{ bottomRight2D.x, bottomRight2D.y },
							};
						}
			return {};
		}

	private:
		const seir::Vec2 _viewportSize;
		const seir::Vec3 _cameraPosition;
		const seir::Mat4 _viewMatrix;
		const seir::Mat4 _inverseViewMatrix = inverse(_viewMatrix);
	};

	struct Assets
	{
		seir::SharedPtr<seir::Texture2D> _boardTexture;
		seir::SharedPtr<seir::Mesh> _boardMesh;
		seir::SharedPtr<seir::ShaderSet> _boardShaders;
		seir::SharedPtr<seir::Mesh> _cubeMesh;
		seir::SharedPtr<seir::ShaderSet> _cubeShaders;

		explicit Assets(seir::Renderer& renderer)
			: _boardTexture{ ::makeBgra32(renderer, 128, 128, [](size_t x, size_t y) {
				return ((x ^ y) & 1) ? seir::Rgba32::grayscale(0xdd) : seir::Rgba32::black();
			}) }
			, _boardMesh{ renderer.createMesh(seir::MeshData::load(seir::fromFile(LOCAL_DATA_DIR "board.obj"))) }
			, _boardShaders{ renderer.createShaders(kVertexShaderV3N3T2, kFragmentShaderV3N3T2) }
			, _cubeMesh{ renderer.createMesh(seir::MeshData::load(seir::fromFile(LOCAL_DATA_DIR "cube.obj"))) }
			, _cubeShaders{ renderer.createShaders(kVertexShaderV3N3, kFragmentShaderV3N3) }
		{
		}
	};

	class Example
	{
	public:
		void present(seir::GuiFrame&& frame)
		{
			const seir::Plane kBoardPlane{ { 0, 0, 1 }, { 0, 0, 0 } };

			// TODO: Framerate-independent movement.
			if (const auto left = frame.takeKeyState(seir::Key::Left); left && *left)
				_cameraPosition.x = std::max(_cameraPosition.x - .125f, -54.f);
			if (const auto right = frame.takeKeyState(seir::Key::Right); right && *right)
				_cameraPosition.x = std::min(_cameraPosition.x + .125f, 54.f);
			if (const auto down = frame.takeKeyState(seir::Key::Down); down && *down)
				_cameraPosition.y = std::max(_cameraPosition.y - .125f, -67.f);
			if (const auto up = frame.takeKeyState(seir::Key::Up); up && *up)
				_cameraPosition.y = std::min(_cameraPosition.y + .125f, 46.f);

			const Camera3D camera{ frame.size(), _cameraPosition };
			_viewMatrix = camera.viewMatrix();
			presentMinimap(frame, camera.viewportQuad(kBoardPlane, {}));
			{
				seir::GuiLayout layout{ frame };
				updateBoardCell(camera, kBoardPlane, frame.addHoverArea(frame.size()));
			}
			presentDebugGraphics(frame);
			if (frame.takeKeyPress(seir::Key::Escape))
				frame.close();
		}

		void render(seir::RenderPass& pass, const Assets& assets)
		{
			pass.updateUniformBuffer(_viewMatrix);
			pass.bindShaders(assets._boardShaders);
			pass.bindTexture(assets._boardTexture);
			pass.bindUniformBuffer(true);
			pass.setTransformation(seir::Mat4::identity());
			pass.drawMesh(*assets._boardMesh);
			if (_boardCell)
			{
				pass.bindShaders(assets._cubeShaders);
				pass.setTransformation(seir::Mat4::translation({ _boardCell->x + .5f, _boardCell->y + .5f, .5 }));
				pass.drawMesh(*assets._cubeMesh);
			}
		}

		void setFps(float fps)
		{
			_fps.clear();
			std::format_to(std::back_inserter(_fps), "{:.1f} fps", fps);
		}

	private:
		void presentDebugGraphics(seir::GuiFrame& frame) // TODO: Refactor out debug text and make this function const.
		{
			_debugText.clear();
			std::format_to(std::back_inserter(_debugText), "cam={:+.1f},{:+.1f}", _cameraPosition.x, _cameraPosition.y);
			if (_boardCell)
				std::format_to(std::back_inserter(_debugText), " cur={:+},{:+}", _boardCell->x, _boardCell->y);
			{
				auto& renderer = frame.renderer();
				renderer.setColor(seir::Rgba32::black(0xaa));
				renderer.setTexture({});
				renderer.addRect({ { 0, 0 }, seir::SizeF{ frame.size()._width, 20 } });
			}
			frame.setLabelStyle({ seir::Rgba32::white(), 1 });
			seir::GuiLayout layout{ frame };
			layout.setItemSize({ 0, 16 });
			layout.fromTopLeft(seir::GuiLayout::Axis::Y, 4);
			frame.addLabel(_debugText, seir::GuiAlignment::Left);
			layout.fromTopRight(seir::GuiLayout::Axis::Y, 4);
			frame.addLabel(_fps, seir::GuiAlignment::Right);
		}

		void presentMinimap(seir::GuiFrame& frame, const seir::QuadF& cameraQuad) const
		{
			const seir::SizeF minimapSize{ 160, 160 };
			const seir::Vec2 minimapScale{ minimapSize._width / 128, minimapSize._height / -128 };
			seir::GuiLayout layout{ frame };
			layout.fromBottomRight(seir::GuiLayout::Axis::Y, 8);
			const auto minimapRect = layout.addItem(minimapSize);
			const auto minimapCenter = minimapRect.center();
			auto& renderer = frame.renderer();
			renderer.setTexture({});
			renderer.setColor(seir::Rgba32::white(0x55));
			renderer.addRect(minimapRect);
			renderer.setColor(seir::Rgba32::red(0xaa));
			renderer.addQuad({
				minimapCenter + cameraQuad._a * minimapScale,
				minimapCenter + cameraQuad._b * minimapScale,
				minimapCenter + cameraQuad._c * minimapScale,
				minimapCenter + cameraQuad._d * minimapScale,
			});
		}

		void updateBoardCell(const Camera3D& camera, const seir::Plane& boardPlane, const std::optional<seir::Vec2>& cursor)
		{
			if (const auto boardPoint = camera.pixelRayIntersection(cursor, boardPlane);
				boardPoint && std::abs(boardPoint->x) <= 64 && std::abs(boardPoint->y) <= 64)
				_boardCell.emplace(std::floor(boardPoint->x), std::floor(boardPoint->y));
			else
				_boardCell.reset();
		}

	private:
		seir::Vec3 _cameraPosition{ 0, -8.5, 16 };
		seir::Mat4 _viewMatrix;
		std::optional<seir::Vec2> _cursor;
		std::string _fps;
		std::string _debugText;
		std::optional<seir::Vec2> _boardCell;
	};
}

int u8main(int, char**)
{
	seir::App app;
	seir::Window window{ app, "Board" };
	seir::Renderer renderer{ window };
	Assets assets{ renderer };
	seir::Renderer2D renderer2d;
	seir::GuiContext gui{ window, seir::Font::load(renderer, seir::fromFile(SEIR_DATA_DIR "fonts/SourceCodePro-Regular.ttf"), 16) };
	Example example;
	for (seir::VariableRate clock; app.processEvents(gui.eventCallbacks());)
	{
		example.present({ gui, renderer2d });
		renderer.render([&](seir::RenderPass& pass) {
			example.render(pass, assets);
			renderer2d.draw(pass);
		});
		if (const auto period = clock.advance())
			example.setFps(period->_averageFrameRate);
	}
	return 0;
}
