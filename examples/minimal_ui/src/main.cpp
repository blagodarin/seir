// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_app/app.hpp>
#include <seir_app/window.hpp>
#include <seir_base/clock.hpp>
#include <seir_graphics/color.hpp>
#include <seir_io/inlet.hpp>
#include <seir_renderer/canvas.hpp>
#include <seir_renderer/renderer.hpp>
#include <seir_u8main/u8main.hpp>
#include <seir_ui/context.hpp>
#include <seir_ui/font.hpp>
#include <seir_ui/frame.hpp>
#include <seir_ui/layout.hpp>
#include <seir_ui/style.hpp>

#include <format>

namespace
{
	class Example
	{
	public:
		void present(seir::UiFrame&& ui)
		{
			seir::UiLayout layout{ ui };
			layout.fromTopRight(seir::UiLayout::Axis::Y, 4);
			layout.setItemSize({ 0, 16 });
			ui.setLabelStyle({ seir::Rgba32::white(), 1 });
			ui.addLabel(_fps, seir::UiAlignment::Right);
			layout.downFromCenter(16);
			ui.addLabel("Press ESC to quit", seir::UiAlignment::Center);
			if (ui.takeKeyPress(seir::Key::Escape))
				ui.close();
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
	seir::Window window{ app, "Minimal UI" };
	seir::Renderer renderer{ window };
	seir::Canvas canvas;
	seir::UiContext uiContext{ window, seir::Font::load(renderer, seir::fromFile(SEIR_DATA_DIR "fonts/SourceCodePro-Regular.ttf"), 16) };
	Example example;
	for (seir::VariableRate clock; app.processEvents(uiContext.eventCallbacks());)
	{
		if (const auto period = clock.advance())
			example.setFps(period->_averageFrameRate);
		example.present({ uiContext, canvas });
		renderer.render([&](seir::RenderPass& pass) {
			canvas.render(pass);
		});
	}
	return 0;
}
