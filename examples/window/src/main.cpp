// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_app/app.hpp>
#include <seir_app/events.hpp>
#include <seir_app/window.hpp>
#include <seir_math/vec.hpp>
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

	struct EventCallbacks : seir::EventCallbacks
	{
		void onKeyEvent(seir::Window& window, const seir::KeyEvent& event) override
		{
			if (event._key == seir::Key::Escape && event._pressed)
				window.close();
		}
	};

	class FpsCounter
	{
	public:
		std::optional<float> updateFps() noexcept
		{
			++_frames;
			const auto currentTime = Clock::now();
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
		Clock::time_point _baseTime = Clock::now();
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
	FpsCounter fpsCounter;
	for (EventCallbacks callbacks; app->processEvents(callbacks);)
	{
		renderer->draw(*mesh);
		if (const auto fps = fpsCounter.updateFps())
			window->setTitle(fmt::format("Window [{:.1f} fps]", *fps));
	}
	return 0;
}
