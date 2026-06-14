// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_app/app.hpp>
#include <seir_app/window.hpp>
#include <seir_base/clock.hpp>
#include <seir_graphics/camera_view.hpp>
#include <seir_graphics/color.hpp>
#include <seir_image/image.hpp>
#include <seir_io/inlet.hpp>
#include <seir_math/euler.hpp>
#include <seir_math/mat.hpp>
#include <seir_math/plane.hpp>
#include <seir_math/ray.hpp>
#include <seir_model/mesh_data.hpp>
#include <seir_renderer/canvas.hpp>
#include <seir_renderer/renderer.hpp>
#include <seir_u8main/u8main.hpp>
#include <seir_ui/context.hpp>
#include <seir_ui/font.hpp>
#include <seir_ui/frame.hpp>
#include <seir_ui/layout.hpp>
#include <seir_ui/style.hpp>

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

	constexpr seir::SizeF kBoardSize{ 128, 128 };

	class CameraManager
	{
	public:
		CameraManager(const seir::Plane& groundPlane)
			: _groundPlane{ groundPlane } {}

		const seir::Plane& groundPlane() const noexcept { return _groundPlane; }
		const seir::Vec3& position() const noexcept { return _cameraPosition; }
		const seir::CameraView& view() const noexcept { return _cameraView; }

		void present(seir::UiFrame& ui, unsigned duration)
		{
			updateCameraPosition(ui, duration);
			_cameraView = { ui.size(), _cameraPosition, { 0, -60, 0 }, 35, .5 };
			drawMinimap(ui, _cameraView.viewportProjection(_groundPlane, {}));
		}

	private:
		static void drawMinimap(seir::UiFrame& ui, const seir::QuadF& viewportProjection)
		{
			seir::UiLayout layout{ ui };
			layout.fromBottomRight(seir::UiLayout::Axis::Y, 8);
			const auto minimapRect = layout.addItem(kMinimapSize);
			const auto minimapCenter = minimapRect.center();
			auto& canvas = ui.canvas();
			canvas.setTexture({});
			canvas.setColor(seir::Rgba32::white(0x55));
			canvas.drawRect(minimapRect);
			canvas.setColor(seir::Rgba32::red(0xaa));
			constexpr seir::Vec2 minimapScale{
				kMinimapSize._width / kBoardSize._width,
				kMinimapSize._height / -kBoardSize._height,
			};
			canvas.drawQuad({
				viewportProjection._a * minimapScale + minimapCenter,
				viewportProjection._b * minimapScale + minimapCenter,
				viewportProjection._c * minimapScale + minimapCenter,
				viewportProjection._d * minimapScale + minimapCenter,
			});
		}

		static std::optional<seir::Vec2> takeMinimapPosition(seir::UiFrame& ui)
		{
			seir::UiLayout layout{ ui };
			layout.fromBottomRight(seir::UiLayout::Axis::Y, 8);
			const auto cursor = ui.addDragArea("minimap", kMinimapSize, seir::Key::Mouse1);
			if (!cursor)
				return {};
			layout.fromBottomRight(seir::UiLayout::Axis::Y, 8);
			const auto minimapRect = layout.addItem(kMinimapSize);
			return seir::Vec2{
				(cursor->x - minimapRect.left()) / minimapRect.width() * kBoardSize._width - kBoardSize._width / 2,
				(minimapRect.top() - cursor->y) / minimapRect.height() * kBoardSize._height + kBoardSize._height / 2,
			};
		}

		void updateCameraPosition(seir::UiFrame& ui, unsigned duration)
		{
			constexpr float kBorderWidth = 2;
			const auto forcedPosition = takeMinimapPosition(ui); // Should happen before border hover takes the mouse.
			const auto borderHover = ui.takeBorderHover(kBorderWidth);
			const auto left = ui.takeKeyDown(seir::Key::Left) || borderHover.x < 0;
			const auto right = ui.takeKeyDown(seir::Key::Right) || borderHover.x > 0;
			const auto down = ui.takeKeyDown(seir::Key::Down) || borderHover.y > 0;
			const auto up = ui.takeKeyDown(seir::Key::Up) || borderHover.y < 0;
			if (forcedPosition)
			{
				const auto bounded = kCameraBound.bound({ forcedPosition->x, forcedPosition->y + kCameraOffsetY });
				_cameraPosition.x = bounded.x;
				_cameraPosition.y = bounded.y;
			}
			else
			{
				constexpr auto kCameraSpeed = 10.f;
				const auto step = static_cast<float>(duration) / 1000.f * kCameraSpeed;
				if (left != right)
				{
					if (left)
						_cameraPosition.x = std::max(_cameraPosition.x - step, kCameraBound.left());
					else
						_cameraPosition.x = std::min(_cameraPosition.x + step, kCameraBound.right());
				}
				if (down != up)
				{
					if (down)
						_cameraPosition.y = std::max(_cameraPosition.y - step, kCameraBound.top());
					else
						_cameraPosition.y = std::min(_cameraPosition.y + step, kCameraBound.bottom());
				}
			}
		}

	private:
		seir::Vec3 _cameraPosition{ 0, kCameraOffsetY, 16 };
		seir::CameraView _cameraView;
		seir::Plane _groundPlane;

		static constexpr seir::RectF kCameraBound{ { -54.f, -67.f }, seir::Vec2{ 54.f, 46.f } };
		static constexpr float kCameraOffsetY = -8.5;
		static constexpr seir::SizeF kMinimapSize{ 160, 160 };
	};

	class Example
	{
	public:
		void present(seir::UiFrame&& ui, unsigned duration)
		{
			_camera.present(ui, duration);
			updateBoardCell(ui);
			drawDebugGraphics(ui);
			if (ui.takeKeyPress(seir::Key::Escape))
				ui.close();
		}

		void render(seir::RenderPass& pass, const Assets& assets)
		{
			pass.updateUniformBuffer(_camera.view().matrix());
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
		void drawDebugGraphics(seir::UiFrame& ui)
		{
			_debugText.clear();
			std::format_to(std::back_inserter(_debugText), "cam={:+.1f},{:+.1f}", _camera.position().x, _camera.position().y);
			if (_boardCell)
				std::format_to(std::back_inserter(_debugText), " cur={:+},{:+}", _boardCell->x, _boardCell->y);
			{
				auto& canvas = ui.canvas();
				canvas.setColor(seir::Rgba32::black(0xaa));
				canvas.setTexture({});
				canvas.drawRect({ { 0, 0 }, seir::SizeF{ ui.size()._width, 20 } });
			}
			ui.setLabelStyle({ seir::Rgba32::white(), 1 });
			seir::UiLayout layout{ ui };
			layout.setItemSize({ 0, 16 });
			layout.fromTopLeft(seir::UiLayout::Axis::Y, 4);
			ui.addLabel(_debugText, seir::UiAlignment::Left);
			layout.fromTopRight(seir::UiLayout::Axis::Y, 4);
			ui.addLabel(_fps, seir::UiAlignment::Right);
		}

		void updateBoardCell(seir::UiFrame& ui)
		{
			seir::UiLayout layout{ ui };
			const auto cursor = ui.addHoverArea(ui.size());
			if (const auto boardPoint = _camera.view().pixelRayIntersection(cursor, _camera.groundPlane());
				boardPoint && std::abs(boardPoint->x) <= 64 && std::abs(boardPoint->y) <= 64)
				_boardCell.emplace(std::floor(boardPoint->x), std::floor(boardPoint->y));
			else
				_boardCell.reset();
		}

	private:
		CameraManager _camera{ { { 0, 0, 1 }, {} } };
		std::optional<seir::Vec2> _boardCell;
		std::string _fps;
		std::string _debugText;
	};
}

int u8main(int, char**)
{
	seir::App app;
	seir::Window window{ app, "Board" };
	seir::Renderer renderer{ window };
	Assets assets{ renderer };
	seir::Canvas canvas;
	seir::UiContext uiContext{ window, seir::Font::load(renderer, seir::fromFile(SEIR_DATA_DIR "fonts/SourceCodePro-Regular.ttf"), 16) };
	Example example;
	seir::ConstantRate actionClock{ std::chrono::milliseconds{ 1 } };
	seir::VariableRate frameClock;
	while (app.processEvents(uiContext.eventCallbacks()))
	{
		if (const auto period = frameClock.advance())
			example.setFps(period->_averageFrameRate);
		example.present({ uiContext, canvas }, actionClock.advance());
		renderer.render([&](seir::RenderPass& pass) {
			example.render(pass, assets);
			canvas.render(pass);
		});
	}
	return 0;
}
