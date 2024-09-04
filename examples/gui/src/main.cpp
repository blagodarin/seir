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
#include <seir_math/mat.hpp>
#include <seir_renderer/2d.hpp>
#include <seir_renderer/renderer.hpp>
#include <seir_u8main/u8main.hpp>

#include <fmt/core.h>

int u8main(int, char**)
{
	seir::App app;
	seir::Window window{ app, "GUI" };
	seir::Renderer renderer{ window };
	seir::Renderer2D renderer2d;
	seir::GuiContext gui{ window };
	gui.setDefaultFont(seir::Font::load(renderer, seir::Blob::from(SEIR_DATA_DIR "source_sans_pro.ttf"), 24));
	window.show();
	seir::VariableRate clock;
	std::string fps1;
	std::string fps2;
	bool showFps = true;
	while (app.processEvents(gui.eventCallbacks()))
	{
		seir::GuiFrame frame{ gui, renderer2d };
		if (frame.takeKeyPress(seir::Key::Escape))
			window.close();
		renderer.render([&](seir::RenderPass& pass) {
			seir::GuiLayout layout{ frame };
			layout.fromTopRight(seir::GuiLayout::Axis::X, 4);
			layout.setItemSize({ 128, 32 });
			layout.setItemSpacing(5);
			if (frame.addButton("quit", "Quit"))
				window.close();
			if (frame.addButton("fps", showFps ? "Hide FPS" : "Show FPS"))
				showFps = !showFps;
			if (const auto cursor = frame.takeMouseCursor())
			{
				frame.selectWhiteTexture();
				renderer2d.setColor(seir::Rgba32::red());
				renderer2d.addRect({ *cursor, seir::SizeF{ 5, 5 } });
			}
			if (showFps)
			{
				layout.fromTopLeft(seir::GuiLayout::Axis::Y, 4);
				layout.setItemSize({ 0, 24 });
				frame.addLabel(fps1);
				frame.addLabel(fps2);
			}
			renderer2d.draw(pass);
		});
		if (const auto period = clock.advance())
		{
			fps1.clear();
			fmt::format_to(std::back_inserter(fps1), "{:.1f} fps", period->_averageFrameRate);
			fps2.clear();
			fmt::format_to(std::back_inserter(fps2), "{:.1f} < {} ms/frame", 1000 / period->_averageFrameRate, period->_maxFrameDuration);
		}
	}
	return 0;
}
