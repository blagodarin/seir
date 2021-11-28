// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_audio/player.hpp>

#include <seir_audio/decoder.hpp>
#include "backend.hpp"
#include "mixer.hpp"

#include <cassert>
#include <cstdio>
#include <mutex>
#include <thread>
#include <vector>

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
			std::scoped_lock lock{ _mutex };
			if (const auto i = std::find_if(_decoders.begin(), _decoders.end(), [&decoder](const auto& item) { return item.first == decoder; }); i != _decoders.end())
				i->second = false;
			else
				_decoders.emplace_back(decoder, false);
		}

		void stop(const seir::SharedPtr<const seir::AudioDecoder>& decoder) noexcept override
		{
			std::scoped_lock lock{ _mutex };
			if (const auto i = std::find_if(_decoders.begin(), _decoders.end(), [&decoder](const auto& item) { return item.first == decoder; }); i != _decoders.end())
			{
				if (const auto last = std::prev(_decoders.end()); i != last)
					std::iter_swap(i, last);
				_decoders.pop_back();
			}
		}

		void stopAll() noexcept override
		{
			std::scoped_lock lock{ _mutex };
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
			std::string message;
			if (description.empty())
			{
				constexpr auto pattern = "[%s] Error 0x%08X";
				message.resize(static_cast<size_t>(std::snprintf(nullptr, 0, pattern, function, code)), '\0');
				std::snprintf(message.data(), message.size() + 1, pattern, function, code);
			}
			else
			{
				constexpr auto pattern = "[%s] Error 0x%08X: %s";
				message.resize(static_cast<size_t>(std::snprintf(nullptr, 0, pattern, function, code, description.c_str())), '\0');
				std::snprintf(message.data(), message.size() + 1, pattern, function, code, description.c_str());
			}
			_callbacks.onPlaybackError(std::move(message));
		}

		bool onBackendIdle() override
		{
			if (_done.load())
				return false;
			const auto wasEmpty = _activeDecoders.empty();
			_activeDecoders.clear();
			{
				std::scoped_lock lock{ _mutex };
				_decoders.erase(
					std::remove_if(_decoders.begin(), _decoders.end(),
						[this](auto& element) {
							if (!element.second)
							{
								element.first->seek(0);
								element.second = true;
							}
							if (element.first->finished())
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
			size_t mixedFrames = 0;
			for (const auto& decoder : _activeDecoders)
				if (const auto frames = _mixer.mix(output, maxFrames, !mixedFrames, *decoder); frames > mixedFrames)
					mixedFrames = frames;
			return mixedFrames;
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
