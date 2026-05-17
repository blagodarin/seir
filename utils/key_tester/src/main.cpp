// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_app/app.hpp>
#include <seir_app/events.hpp>
#include <seir_app/window.hpp>
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

#include <cassert>
#include <deque>
#include <format>

namespace
{
	std::string_view keyToString(seir::Key key)
	{
		using seir::Key;
		switch (key)
		{
		case Key::None: return "None";
		case Key::A: return "A";
		case Key::B: return "B";
		case Key::C: return "C";
		case Key::D: return "D";
		case Key::E: return "E";
		case Key::F: return "F";
		case Key::G: return "G";
		case Key::H: return "H";
		case Key::I: return "I";
		case Key::J: return "J";
		case Key::K: return "K";
		case Key::L: return "L";
		case Key::M: return "M";
		case Key::N: return "N";
		case Key::O: return "O";
		case Key::P: return "P";
		case Key::Q: return "Q";
		case Key::R: return "R";
		case Key::S: return "S";
		case Key::T: return "T";
		case Key::U: return "U";
		case Key::V: return "V";
		case Key::W: return "W";
		case Key::X: return "X";
		case Key::Y: return "Y";
		case Key::Z: return "Z";
		case Key::_1: return "1";
		case Key::_2: return "2";
		case Key::_3: return "3";
		case Key::_4: return "4";
		case Key::_5: return "5";
		case Key::_6: return "6";
		case Key::_7: return "7";
		case Key::_8: return "8";
		case Key::_9: return "9";
		case Key::_0: return "0";
		case Key::Enter: return "Enter";
		case Key::Escape: return "Escape";
		case Key::Backspace: return "Backspace";
		case Key::Tab: return "Tab";
		case Key::Space: return "Space";
		case Key::Minus: return "Minus";
		case Key::Equals: return "Equals";
		case Key::LBracket: return "LBracket";
		case Key::RBracket: return "RBracket";
		case Key::Backslash: return "Backslash";
		case Key::Hash: return "Hash";
		case Key::Semicolon: return "Semicolon";
		case Key::Apostrophe: return "Apostrophe";
		case Key::Grave: return "Grave";
		case Key::Comma: return "Comma";
		case Key::Period: return "Period";
		case Key::Slash: return "Slash";
		case Key::CapsLock: return "CapsLock";
		case Key::F1: return "F1";
		case Key::F2: return "F2";
		case Key::F3: return "F3";
		case Key::F4: return "F4";
		case Key::F5: return "F5";
		case Key::F6: return "F6";
		case Key::F7: return "F7";
		case Key::F8: return "F8";
		case Key::F9: return "F9";
		case Key::F10: return "F10";
		case Key::F11: return "F11";
		case Key::F12: return "F12";
		case Key::PrintScreen: return "PrintScreen";
		case Key::ScrollLock: return "ScrollLock";
		case Key::Pause: return "Pause";
		case Key::Insert: return "Insert";
		case Key::Home: return "Home";
		case Key::PageUp: return "PageUp";
		case Key::Delete: return "Delete";
		case Key::End: return "End";
		case Key::PageDown: return "PageDown";
		case Key::Right: return "Right";
		case Key::Left: return "Left";
		case Key::Down: return "Down";
		case Key::Up: return "Up";
		case Key::NumLock: return "NumLock";
		case Key::Divide: return "Divide";
		case Key::Multiply: return "Multiply";
		case Key::Subtract: return "Subtract";
		case Key::Add: return "Add";
		case Key::NumEnter: return "NumEnter";
		case Key::Num1: return "Num1";
		case Key::Num2: return "Num2";
		case Key::Num3: return "Num3";
		case Key::Num4: return "Num4";
		case Key::Num5: return "Num5";
		case Key::Num6: return "Num6";
		case Key::Num7: return "Num7";
		case Key::Num8: return "Num8";
		case Key::Num9: return "Num9";
		case Key::Num0: return "Num0";
		case Key::Decimal: return "Decimal";
		case Key::NonUsBackslash: return "NonUsBackslash";
		case Key::App: return "App";
		case Key::LControl: return "LControl";
		case Key::LShift: return "LShift";
		case Key::LAlt: return "LAlt";
		case Key::LGui: return "LGui";
		case Key::RControl: return "RControl";
		case Key::RShift: return "RShift";
		case Key::RAlt: return "RAlt";
		case Key::RGui: return "RGui";
		case Key::Mouse1: return "Mouse1";
		case Key::Mouse2: return "Mouse2";
		case Key::Mouse3: return "Mouse3";
		case Key::Mouse4: return "Mouse4";
		case Key::Mouse5: return "Mouse5";
		}
		return {};
	}

	class KeyHandler : public seir::EventCallbacks
	{
	public:
		void onKeyEvent(seir::Window&, const seir::KeyEvent& event) override
		{
			if (_events.size() == 100)
				_events.pop_back();
			_events.emplace_front(std::format("{} {}{}",
				event._pressed ? "+" : "-",
				event._shiftPressed ? "Shift+" : "",
				::keyToString(event._key)));
		}

		void onTextEvent(seir::Window&, std::string_view text) override
		{
			assert(!_events.empty());
			std::string textBytes;
			for (const auto c : text)
			{
				if (!textBytes.empty())
					textBytes += ' ';
				std::format_to(std::back_inserter(textBytes), "{:02x}", c);
			}
			_events.front() += std::format(" \"{}\" ({})", text, textBytes);
		}

		const auto& events() const { return _events; }

	private:
		std::deque<std::string> _events;
	};
}

int u8main(int, char**)
{
	seir::App app;
	seir::Window window{ app, "Key Tester" };
	seir::Renderer renderer{ window };
	seir::Renderer2D renderer2d;
	seir::GuiContext gui{ window };
	gui.setDefaultFont(seir::Font::load(renderer, seir::Blob::from(SEIR_DATA_DIR "fonts/SourceCodePro-Regular.ttf"), 24));
	window.show();
	for (KeyHandler handler; app.processEvents(handler);)
	{
		{
			seir::GuiFrame frame{ gui, renderer2d };
			seir::GuiLayout layout{ frame };
			layout.fromBottomLeft(seir::GuiLayout::Axis::Y, 2);
			layout.setItemSize({ 0, 24 });
			layout.setItemSpacing(0);
			frame.setLabelStyle({ seir::Rgba32::white(), 1 });
			for (const auto& event : handler.events())
				frame.addLabel(event);
		}
		renderer.render([&](seir::RenderPass& pass) {
			renderer2d.draw(pass);
		});
	}
	return 0;
}
