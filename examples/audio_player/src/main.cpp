// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_audio/decoder.hpp>
#include <seir_audio/format.hpp>
#include <seir_audio/player.hpp>
#include <seir_data/blob.hpp>
#include <seir_data/file.hpp>

#include <cassert>
#include <condition_variable>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <optional>

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
		std::cerr << "Usage:\n\t" << std::filesystem::path{ argv[0] }.filename().string() << " FILE\n";
		return 1;
	}
	auto decoder = seir::AudioDecoder::create(seir::createFileBlob(argv[1]));
	if (!decoder)
	{
		std::cerr << "Unable to play " << argv[1] << '\n';
		return 1;
	}
	AudioCallbacks callbacks;
	const auto player = seir::AudioPlayer::create(callbacks, decoder->format().samplingRate());
	assert(player);
	player->play(seir::SharedPtr{ std::move(decoder) });
	return callbacks.join();
}
