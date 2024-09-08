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
#include <seir_gui/context.hpp>
#include <seir_gui/font.hpp>
#include <seir_gui/frame.hpp>
#include <seir_gui/layout.hpp>
#include <seir_gui/style.hpp>
#include <seir_math/mat.hpp>
#include <seir_renderer/2d.hpp>
#include <seir_renderer/renderer.hpp>
#include <seir_u8main/u8main.hpp>

#include <fmt/core.h>

class Example
{
public:
	Example(seir::GuiContext& gui, seir::Renderer& renderer) // TODO: Replace renderer with some asset manager.
	{
		gui.setDefaultFont(seir::Font::load(renderer, seir::Blob::from(SEIR_DATA_DIR "source_sans_pro.ttf"), 24));
	}

	bool presentGui(seir::GuiFrame&& frame)
	{
		bool quit = false;
		if (frame.takeKeyPress(seir::Key::Escape))
			quit = true;
		seir::GuiLayout layout{ frame };
		layout.fromTopRight(seir::GuiLayout::Axis::X, 4);
		layout.setItemSize({ 128, 32 });
		layout.setItemSpacing(4);
		if (frame.addButton("quit", "Quit"))
			quit = true;
		if (frame.addButton("fps", _showFps ? "Hide FPS" : "Show FPS"))
			_showFps = !_showFps;
		if (frame.addStringEdit("input", _input))
		{
			_output = std::move(_input);
			_input.clear();
		}
		frame.addLabel(_output);
		if (const auto cursor = frame.takeMouseCursor())
		{
			frame.selectWhiteTexture();
			frame.renderer().setColor(seir::Rgba32::red());
			frame.renderer().addRect({ *cursor, seir::SizeF{ 5, 5 } });
		}
		if (_showFps)
		{
			layout.fromTopLeft(seir::GuiLayout::Axis::Y, 2);
			layout.setItemSize({ 0, 24 });
			layout.setItemSpacing(0);
			frame.setLabelStyle({ seir::Rgba32::white(), 1 });
			frame.addLabel(_fps1);
			frame.addLabel(_fps2);
		}
		return !quit; // TODO: Consider signaling this through frame object.
	}

	void setFps(const seir::VariablePeriod& period)
	{
		_fps1.clear();
		fmt::format_to(std::back_inserter(_fps1), "{:.1f} fps", period._averageFrameRate);
		_fps2.clear();
		fmt::format_to(std::back_inserter(_fps2), "{:.1f} < {} ms/frame", 1000 / period._averageFrameRate, period._maxFrameDuration);
	}

private:
	bool _showFps = true;
	std::string _fps1;
	std::string _fps2;
	std::string _input;
	std::string _output;
};

int u8main(int, char**)
{
	seir::App app;
	seir::Window window{ app, "GUI" };
	seir::Renderer renderer{ window };
	seir::Renderer2D renderer2d;
	seir::GuiContext gui{ window };
	Example example{ gui, renderer };
	window.show();
	for (seir::VariableRate clock; app.processEvents(gui.eventCallbacks());)
	{
		if (!example.presentGui({ gui, renderer2d }))
			window.close();
		renderer.render([&](seir::RenderPass& pass) {
			renderer2d.draw(pass);
		});
		if (const auto period = clock.advance())
			example.setFps(*period);
	}
	return 0;
}
