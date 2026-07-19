// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_app/app.hpp>
#include <seir_app/window.hpp>
#include <seir_audio/decoder.hpp>
#include <seir_audio/format.hpp>
#include <seir_audio/player.hpp>
#include <seir_io/directory_view.hpp>
#include <seir_io/inlet.hpp>
#include <seir_renderer/canvas.hpp>
#include <seir_renderer/color.hpp>
#include <seir_renderer/renderer.hpp>
#include <seir_u8main/u8main.hpp>
#include <seir_ui/context.hpp>
#include <seir_ui/font.hpp>
#include <seir_ui/frame.hpp>
#include <seir_ui/layout.hpp>
#include <seir_ui/style.hpp>

#include <cassert>

namespace
{
	class Controller
	{
	public:
		Controller(seir::AudioPlayer& player)
			: _player{ player }
		{
			constexpr auto dirTextColor = seir::Rgba32::yellow();
			constexpr auto fileTextColor = seir::Rgba32::white();

			_dirButtonStyle._fontSize = 1.f;
			_dirButtonStyle._normal._backgroundColor = seir::Rgba32::grayscale(0x04, 0xf0);
			_dirButtonStyle._normal._textColor = dirTextColor;
			_dirButtonStyle._hovered._backgroundColor = seir::Rgba32::grayscale(0x08, 0xf0);
			_dirButtonStyle._hovered._textColor = dirTextColor;
			_dirButtonStyle._pressed._backgroundColor = seir::Rgba32::grayscale(0x10, 0xf0);
			_dirButtonStyle._pressed._textColor = dirTextColor;

			_fileButtonStyle._fontSize = 1.f;
			_fileButtonStyle._normal._backgroundColor = _dirButtonStyle._normal._backgroundColor;
			_fileButtonStyle._normal._textColor = fileTextColor;
			_fileButtonStyle._hovered._backgroundColor = _dirButtonStyle._hovered._backgroundColor;
			_fileButtonStyle._hovered._textColor = fileTextColor;
			_fileButtonStyle._pressed._backgroundColor = _dirButtonStyle._pressed._backgroundColor;
			_fileButtonStyle._pressed._textColor = fileTextColor;
		}

		void present(seir::UiFrame&& ui)
		{
			seir::UiLayout layout{ ui };

			layout.fromTopLeft(seir::UiLayout::Axis::Y, 6);
			layout.setItemSize({ 0, 20 });
			ui.setLabelStyle({ seir::Rgba32::green(), 1 });
			ui.addLabel(_directoryView.path(), seir::UiAlignment::Left);

			layout.fromTopRight(seir::UiLayout::Axis::Y, 4);
			layout.setItemSize({ 96, 24 });
			if (ui.addButton("stop", "Stop"))
				_player.stopAll();

			layout.fromTopLeft(seir::UiLayout::Axis::Y, 4);
			layout.skip(28);
			layout.setItemSize({ 792, 20 });
			{
				std::filesystem::path newPath;
				for (const auto& entry : _directoryView.entries())
				{
					if (entry.type == seir::DirectoryView::Entry::Type::Directory)
					{
						ui.setButtonStyle(_dirButtonStyle);
						if (ui.addButton(entry.fullName, entry.name, seir::UiAlignment::Left))
							newPath = entry.path;
					}
					else
					{
						ui.setButtonStyle(_fileButtonStyle);
						if (ui.addButton(entry.fullName, entry.name, seir::UiAlignment::Left))
						{
							_player.stopAll();
							if (auto decoder = seir::AudioDecoder::create(seir::fromFile(entry.fullName), { .format{}, .loop = true }))
								_player.play(seir::SharedPtr{ std::move(decoder) });
						}
					}
				}
				if (!newPath.empty())
					_directoryView.reset(newPath);
			}

			if (ui.takeKeyPress(seir::Key::Escape))
				ui.close();
		}

	private:
		seir::AudioPlayer& _player;
		seir::DirectoryView _directoryView{ "*.aulos" };
		seir::UiButtonStyle _dirButtonStyle;
		seir::UiButtonStyle _fileButtonStyle;
	};

	class AudioCallbacks : public seir::AudioCallbacks
	{
	private:
		void onPlaybackError(seir::AudioError) override {}
		void onPlaybackError(std::string&&) override {}
		void onPlaybackStarted() override {}
		void onPlaybackStopped() override {}
	};
}

int u8main(int, char**)
{
	seir::App app;
	seir::Window window{ app, "Audio Player" };
	seir::Renderer renderer{ window };
	seir::Canvas canvas;
	seir::UiContext uiContext{ window, seir::Font::load(renderer, seir::fromFile(SEIR_DATA_DIR "fonts/SourceCodePro-Regular.ttf"), 20) };
	AudioCallbacks callbacks;
	const auto player = seir::AudioPlayer::create(callbacks);
	assert(player);
	Controller controller{ *player };
	while (app.processEvents(uiContext.eventCallbacks()))
	{
		controller.present({ uiContext, canvas });
		renderer.render([&](seir::RenderPass& pass) {
			canvas.render(pass);
		});
	}
	return 0;
}
