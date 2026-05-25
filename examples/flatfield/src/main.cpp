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
#include <seir_io/blob.hpp>
#include <seir_renderer/2d.hpp>
#include <seir_renderer/renderer.hpp>
#include <seir_u8main/u8main.hpp>

#include <format>

namespace
{
	class Example
	{
	public:
		void presentGui(seir::GuiFrame&& frame)
		{
			seir::GuiLayout layout{ frame };
			layout.fromTopRight(seir::GuiLayout::Axis::Y, 4);
			layout.setItemSize({ 0, 16 });
			frame.setLabelStyle({ seir::Rgba32::white(), 1 });
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
	seir::Window window{ app, "Flatfield" };
	seir::Renderer renderer{ window };
	seir::Renderer2D renderer2d;
	seir::GuiContext gui{ window, seir::Font::create(renderer, seir::load(SEIR_DATA_DIR "fonts/SourceCodePro-Regular.ttf"), 16) };
	Example example;
	for (seir::VariableRate clock; app.processEvents(gui.eventCallbacks());)
	{
		example.presentGui({ gui, renderer2d });
		renderer.render([&](seir::RenderPass& pass) {
			renderer2d.draw(pass);
		});
		if (const auto period = clock.advance())
			example.setFps(period->_averageFrameRate);
	}
	return 0;
}
