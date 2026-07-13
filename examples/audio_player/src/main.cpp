// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_app/app.hpp>
#include <seir_app/window.hpp>
#include <seir_audio/decoder.hpp>
#include <seir_audio/format.hpp>
#include <seir_audio/player.hpp>
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

#include <algorithm>
#include <cassert>
#include <filesystem>

namespace
{
	class Controller
	{
	public:
		Controller(seir::AudioPlayer& player)
			: _player{ player }
		{
			listPath(std::filesystem::current_path());

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
			ui.addLabel(_currentPath, seir::UiAlignment::Left);

			layout.fromTopRight(seir::UiLayout::Axis::Y, 4);
			layout.setItemSize({ 96, 24 });
			if (ui.addButton("stop", "Stop"))
				_player.stopAll();

			layout.fromTopLeft(seir::UiLayout::Axis::Y, 4);
			layout.skip(28);
			layout.setItemSize({ 792, 20 });
			std::filesystem::path newPath;
			std::string selectedFile;
			for (const auto& entry : _entries)
			{
				if (entry.type == Entry::Type::Directory)
				{
					ui.setButtonStyle(_dirButtonStyle);
					if (ui.addButton(entry.pathString, entry.name, seir::UiAlignment::Left))
						newPath = entry.path;
				}
				else
				{
					ui.setButtonStyle(_fileButtonStyle);
					if (ui.addButton(entry.pathString, entry.name, seir::UiAlignment::Left))
						selectedFile = entry.pathString;
				}
			}
			if (ui.takeKeyPress(seir::Key::Escape))
			{
				ui.close();
				return;
			}

			if (!newPath.empty())
				listPath(newPath);
			if (!selectedFile.empty())
			{
				_player.stopAll();
				if (auto decoder = seir::AudioDecoder::create(seir::fromFile(selectedFile), { .loop = true }))
					_player.play(seir::SharedPtr{ std::move(decoder) });
			}
		}

	private:
		void listPath(const std::filesystem::path& path)
		{
			decltype(_entries) entries;
			try
			{
				if (path.has_parent_path())
					entries.emplace_back(path.parent_path(), Entry::Type::Directory, "..");
				for (const auto& entry : std::filesystem::directory_iterator{ path })
				{
					auto filename = entry.path().filename().string();
					if (filename.starts_with('.'))
						continue;
					if (entry.is_directory())
						entries.emplace_back(std::filesystem::path{ entry.path() }, Entry::Type::Directory, std::move(filename));
					else if (entry.is_regular_file() && filename.ends_with(".aulos"))
						entries.emplace_back(std::filesystem::path{ entry.path() }, Entry::Type::File, std::move(filename));
				}
			}
			catch (const std::filesystem::filesystem_error&)
			{
				return;
			}
			std::sort(path.has_parent_path() ? std::next(entries.begin()) : entries.begin(), entries.end(),
				[](const Entry& left, const Entry& right) {
					if (left.type != right.type)
						return left.type == Entry::Type::Directory;
					return left.name < right.name;
				});
			_currentPath = path.string();
			_entries = std::move(entries);
		}

	private:
		struct Entry
		{
			enum class Type : bool
			{
				File,
				Directory,
			};

			std::string name;
			Type type = Type::File;
			std::filesystem::path path;
			std::string pathString = path.string();

			Entry(std::filesystem::path&& path_, Type type_, std::string&& name_)
				: name{ std::move(name_) }
				, type{ type_ }
				, path{ std::move(path_) }
			{}
		};

		seir::AudioPlayer& _player;
		std::string _currentPath;
		std::vector<Entry> _entries;
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

int u8main(int argc, char** argv)
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
