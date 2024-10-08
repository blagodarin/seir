// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_audio/player.hpp>

#include <seir_audio/decoder.hpp>
#include "backend.hpp"
#include "mixer.hpp"

#include <algorithm>
#include <cassert>
#include <mutex>
#include <thread>
#include <vector>

#include <fmt/format.h>

namespace
{
	class AudioPlayerImpl final
		: public seir::AudioPlayer
		, private seir::AudioBackendCallbacks
	{
	public:
		AudioPlayerImpl(seir::AudioCallbacks& callbacks, unsigned preferredSamplingRate)
			: _callbacks{ callbacks }
			, _thread{ [this, preferredSamplingRate] { runAudioBackend(*this, preferredSamplingRate); } }
		{
		}

		~AudioPlayerImpl() noexcept override
		{
			_done.store(true);
			_thread.join();
		}

		void play(const seir::SharedPtr<seir::AudioDecoder>& decoder) override
		{
			assert(decoder);
			const std::scoped_lock lock{ _mutex };
			if (const auto i = std::find_if(_decoders.begin(), _decoders.end(), [&decoder](const auto& item) { return item.first == decoder; }); i != _decoders.end())
				i->second = false;
			else
				_decoders.emplace_back(decoder, false);
		}

		void stop(const seir::SharedPtr<const seir::AudioDecoder>& decoder) noexcept override
		{
			const std::scoped_lock lock{ _mutex };
			if (const auto i = std::find_if(_decoders.begin(), _decoders.end(), [&decoder](const auto& item) { return item.first == decoder; }); i != _decoders.end())
			{
				if (const auto last = std::prev(_decoders.end()); i != last)
					std::iter_swap(i, last);
				_decoders.pop_back();
			}
		}

		void stopAll() noexcept override
		{
			const std::scoped_lock lock{ _mutex };
			_decoders.clear();
		}

	private:
		void onBackendAvailable(unsigned samplingRate, size_t maxReadFrames) override
		{
			_mixer.reset(samplingRate, maxReadFrames);
		}

		void onBackendError(seir::AudioError error) override
		{
			_callbacks.onPlaybackError(error);
		}

		void onBackendError(const char* function, int code, const std::string& description) override
		{
			_callbacks.onPlaybackError(description.empty()
					? fmt::format("[{}] Error 0x{:08X}", function, code)
					: fmt::format("[{}] Error 0x{:08X}: {}", function, code, description));
		}

		bool onBackendIdle() override
		{
			if (_done.load())
				return false;
			const auto wasEmpty = _activeDecoders.empty();
			_activeDecoders.clear();
			{
				const std::scoped_lock lock{ _mutex };
				_decoders.erase(
					std::remove_if(_decoders.begin(), _decoders.end(),
						[this](auto& element) {
							auto& decoderData = seir::AudioMixer::decoderData(*element.first);
							if (!element.second)
							{
								decoderData._finished = !element.first->seek(0);
								decoderData._resamplingOffset = 0;
								element.second = true;
							}
							if (decoderData._finished)
								return true;
							_activeDecoders.emplace_back(element.first);
							return false;
						}),
					_decoders.end());
			}
			if (wasEmpty && !_activeDecoders.empty())
				_callbacks.onPlaybackStarted();
			if (!wasEmpty && _activeDecoders.empty())
				_callbacks.onPlaybackStopped();
			return true;
		}

		size_t onBackendRead(float* output, size_t maxFrames) noexcept override
		{
			size_t totalFrames = 0;
			for (const auto& decoder : _activeDecoders)
				if (const auto frames = _mixer.mix(output, maxFrames, !totalFrames, *decoder); frames > totalFrames)
					totalFrames = frames;
			return totalFrames;
		}

	private:
		seir::AudioCallbacks& _callbacks;
		seir::AudioMixer _mixer;
		std::atomic<bool> _done{ false };
		std::vector<std::pair<seir::SharedPtr<seir::AudioDecoder>, bool>> _decoders;
		std::vector<seir::SharedPtr<seir::AudioDecoder>> _activeDecoders;
		std::mutex _mutex;
		std::thread _thread;
	};
}

namespace seir
{
	UniquePtr<AudioPlayer> AudioPlayer::create(AudioCallbacks& callbacks, unsigned samplingRate)
	{
		return makeUnique<AudioPlayer, AudioPlayerImpl>(callbacks, samplingRate);
	}
}
