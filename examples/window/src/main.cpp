// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_app/app.hpp>
#include <seir_app/events.hpp>
#include <seir_app/window.hpp>
#include <seir_math/euler.hpp>
#include <seir_math/mat.hpp>
#include <seir_renderer/renderer.hpp>
#include <seir_u8main/u8main.hpp>

#include <array>
#include <chrono>
#include <optional>

#include <fmt/core.h>

namespace
{
	struct Vertex
	{
		seir::Vec3 position;
		seir::Vec3 color;
		seir::Vec2 texCoord;
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

	class State : public seir::EventCallbacks
	{
	public:
		seir::Mat4 cameraMatrix() const noexcept
		{
			return seir::Mat4::camera(_cameraPosition, { 0, -45, 0 });
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
		seir::Vec3 _cameraPosition{ 0, -3, 3 };
	};

	class FrameClock
	{
	public:
		float seconds() const noexcept
		{
			return std::chrono::duration_cast<std::chrono::duration<float, std::chrono::seconds::period>>(_totalOffset).count();
		}

		std::optional<float> advance() noexcept
		{
			++_frames;
			const auto currentTime = Clock::now();
			_totalOffset = currentTime - _startTime;
			const auto duration = currentTime - _baseTime;
			if (duration < std::chrono::seconds{ 1 })
				return {};
			const auto result = _frames * std::chrono::duration_cast<std::chrono::duration<float, Clock::period>>(duration).count() / Clock::period::den;
			_baseTime = currentTime;
			_frames = 0;
			return result;
		}

	private:
		using Clock = std::chrono::steady_clock;
		const Clock::time_point _startTime = Clock::now();
		Clock::duration _totalOffset = Clock::duration::zero();
		Clock::time_point _baseTime = _startTime;
		float _frames = 0;
	};
}

int u8main(int, char**)
{
	// TODO: Make less verbose.
	// Posible options include:
	// - making create() return SharedPtr;
	// - removing SharedPtr and passing App and Window by reference.
	const auto app = seir::SharedPtr{ seir::App::create() };
	const auto window = seir::SharedPtr{ seir::Window::create(app, "Window") };
	const auto renderer = seir::Renderer::create(window);
	const auto mesh = renderer->createMesh(kVertexData.data(), sizeof(Vertex), kVertexData.size(), kIndexData.data(), seir::Mesh::IndexType::U16, kIndexData.size());
	window->show();
	FrameClock clock;
	for (State state; app->processEvents(state);)
	{
		renderer->render([&mesh, &clock, &state](const seir::Vec2& viewportSize, seir::RenderPass& renderPass) {
			renderPass.setProjection(seir::Mat4::projection3D(viewportSize.x / viewportSize.y, 45, 1), state.cameraMatrix());
			renderPass.setTransformation(seir::Mat4::rotation(40 * clock.seconds(), { 0, 0, 1 }));
			renderPass.drawMesh(*mesh);
		});
		if (const auto fps = clock.advance())
			window->setTitle(fmt::format("Window [{:.1f} fps]", *fps));
	}
	return 0;
}
