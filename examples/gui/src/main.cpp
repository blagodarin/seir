// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_app/app.hpp>
#include <seir_app/events.hpp>
#include <seir_app/window.hpp>
#include <seir_base/clock.hpp>
#include <seir_data/blob.hpp>
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
	const auto font = seir::Font::load(renderer, seir::Blob::from(SEIR_DATA_DIR "source_sans_pro.ttf"), 20);
	window.show();
	seir::VariableRate clock;
	std::string fps1;
	std::string fps2;
	while (app.processEvents(gui.eventCallbacks()))
	{
		seir::GuiFrame frame{ gui };
		if (frame.takeKeyPress(seir::Key::Escape))
			window.close();
		renderer.render([&](seir::RenderPass& pass) {
			seir::GuiLayout layout{ frame };
			layout.fromTopLeft(seir::GuiLayout::Axis::Y, 10);
			layout.setItemSize({ 200, 20 });
			layout.setItemSpacing(5);
			font->renderLine(renderer2d, layout.addItem(), fps1);
			font->renderLine(renderer2d, layout.addItem(), fps2);
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
