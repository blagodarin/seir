// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_app/app.hpp>
#include <seir_app/events.hpp>
#include <seir_app/window.hpp>
#include <seir_base/clock.hpp>
#include <seir_graphics/color.hpp>
#include <seir_io/inlet.hpp>
#include <seir_math/mat.hpp>
#include <seir_math/rect.hpp>
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
			layout.fromTopRight(seir::UiLayout::Axis::X, 4);
			layout.setItemSize({ 128, 32 });
			layout.setItemSpacing(4);
			if (ui.addButton("quit", "Quit"))
				ui.close();
			if (ui.addButton("fps", _showFps ? "Hide FPS" : "Show FPS"))
				_showFps = !_showFps;
			layout.advance();
			if (std::exchange(_isFirstFrame, false))
				ui.putKeyboardFocus();
			if (ui.addStringEdit("input", _input))
			{
				_output = std::move(_input);
				_input.clear();
			}
			ui.addLabel(_output);
			if (const auto cursor = ui.takeMouseCursor())
			{
				ui.selectWhiteTexture();
				ui.canvas().setColor(seir::Rgba32::red());
				ui.canvas().drawRect({ *cursor, seir::Size2D{ 5, 5 } });
			}
			if (_showFps)
			{
				layout.fromTopLeft(seir::UiLayout::Axis::Y, 2);
				layout.setItemSize({ 0, 24 });
				layout.setItemSpacing(0);
				ui.setLabelStyle({ seir::Rgba32::white(), 1 });
				ui.addLabel(_fps1);
				ui.addLabel(_fps2);
			}
			if (ui.takeKeyPress(seir::Key::Escape))
				ui.close();
		}

		void setFps(const seir::VariablePeriod& period)
		{
			_fps1.clear();
			std::format_to(std::back_inserter(_fps1), "{:.1f} fps", period._averageFrameRate);
			_fps2.clear();
			std::format_to(std::back_inserter(_fps2), "{:.1f} < {} ms/frame", 1000 / period._averageFrameRate, period._maxFrameDuration);
		}

	private:
		bool _isFirstFrame = true;
		bool _showFps = true;
		std::string _fps1;
		std::string _fps2;
		std::string _input;
		std::string _output;
	};
}

int u8main(int, char**)
{
	seir::App app;
	seir::Window window{ app, "UI" };
	seir::Renderer renderer{ window };
	seir::Canvas canvas;
	seir::UiContext uiContext{ window, seir::Font::load(renderer, seir::fromFile(SEIR_DATA_DIR "fonts/SourceSans3-Regular.ttf"), 24) };
	Example example;
	for (seir::VariableRate clock; app.processEvents(uiContext.eventCallbacks());)
	{
		if (const auto period = clock.advance())
			example.setFps(*period);
		example.present({ uiContext, canvas });
		renderer.render([&](seir::RenderPass& pass) {
			canvas.render(pass);
		});
	}
	return 0;
}
