// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_audio/decoder.hpp>
#include <seir_audio/format.hpp>
#include <seir_audio/player.hpp>
#include <seir_io/inlet.hpp>

#include <cassert>
#include <condition_variable>
#include <filesystem>
#include <mutex>
#include <optional>
#include <print>

namespace
{
	class AudioCallbacks : public seir::AudioCallbacks
	{
	public:
		int join()
		{
			std::unique_lock lock{ _mutex };
			_condition.wait(lock, [this] { return _result; });
			return *_result;
		}

	private:
		void stop(int result)
		{
			{
				std::scoped_lock lock{ _mutex };
				_result.emplace(result);
			}
			_condition.notify_one();
		}

		void onPlaybackError(seir::AudioError) override { stop(1); }
		void onPlaybackError(std::string&&) override { stop(1); }
		void onPlaybackStarted() override {}
		void onPlaybackStopped() override { stop(0); }

	private:
		mutable std::mutex _mutex;
		std::condition_variable _condition;
		std::optional<int> _result;
	};
}

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		std::println(stderr, "Usage:");
		std::println(stderr, "\t{} FILE", std::filesystem::path{ argv[0] }.filename().string());
		return 1;
	}
	auto decoder = seir::AudioDecoder::create(seir::fromFile(argv[1]));
	if (!decoder)
	{
		std::println(stderr, "Unable to play {}", argv[1]);
		return 1;
	}
	AudioCallbacks callbacks;
	const auto player = seir::AudioPlayer::create(callbacks, decoder->format().samplingRate());
	assert(player);
	player->play(seir::SharedPtr{ std::move(decoder) });
	return callbacks.join();
}
