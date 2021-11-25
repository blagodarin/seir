// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_audio/player.hpp>

#include "backend.hpp"

#include <seir_base/buffer.hpp>

#include <atomic>
#include <cstdio>
#include <mutex>
#include <thread>

namespace
{
	class AudioPlayerImpl final
		: public seir::AudioPlayer
		, private seir::AudioBackendCallbacks
	{
	public:
		AudioPlayerImpl(seir::AudioCallbacks& callbacks, unsigned samplingRate)
			: _callbacks{ callbacks }
			, _samplingRate{ samplingRate }
			, _thread{ [this] { runAudioBackend(*this, _samplingRate); } }
		{
		}

		~AudioPlayerImpl() noexcept override
		{
			_done.store(true);
			_thread.join();
		}

		seir::AudioFormat format() const noexcept override
		{
			return { seir::AudioSampleType::f32, seir::AudioChannelLayout::Stereo, _samplingRate };
		}

		void play(const seir::SharedPtr<seir::AudioSource>& source) override
		{
			auto inOut = source && source->format().samplingRate() == _samplingRate ? source : nullptr;
			std::scoped_lock lock{ _mutex };
			_source.swap(inOut);
		}

		void stop() noexcept override
		{
			decltype(_source) out;
			std::scoped_lock lock{ _mutex };
			_source.swap(out);
		}

	private:
		void onBackendAvailable(size_t maxReadFrames) override
		{
			_buffer.reserve(maxReadFrames * format().bytesPerFrame(), false);
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
			if (_started)
				_callbacks.onPlaybackStarted();
			if (_stopped)
				_callbacks.onPlaybackStopped();
			return !_done.load();
		}

		size_t onBackendRead(float* output, size_t maxFrames) noexcept override
		{
			using Conversion = void (*)(void*, const void*, size_t) noexcept;
			size_t frames = 0;
			Conversion conversion = nullptr;
			size_t conversionMultiplier = 1;
			if (std::scoped_lock lock{ _mutex }; _source)
			{
				switch (const auto format = _source->format(); format.channelLayout())
				{
				case seir::AudioChannelLayout::Mono:
					switch (format.sampleType())
					{
					case seir::AudioSampleType::i16:
						frames = _source->read(_buffer.data(), maxFrames);
						conversion = reinterpret_cast<Conversion>(seir::convertSamples2x1D);
						break;
					case seir::AudioSampleType::f32:
						frames = _source->read(_buffer.data(), maxFrames);
						conversion = reinterpret_cast<Conversion>(seir::duplicate1D_32);
						break;
					}
					break;
				case seir::AudioChannelLayout::Stereo:
					switch (format.sampleType())
					{
					case seir::AudioSampleType::i16:
						frames = _source->read(_buffer.data(), maxFrames);
						conversion = reinterpret_cast<Conversion>(seir::convertSamples1D);
						conversionMultiplier = 2;
						break;
					case seir::AudioSampleType::f32:
						frames = _source->read(output, maxFrames);
						break;
					}
					break;
				}
				if (frames < maxFrames)
					_source.reset();
			}
			if (conversion)
				conversion(output, _buffer.data(), frames * conversionMultiplier);
			_started = !_playing && frames > 0;
			_stopped = _playing && frames < maxFrames;
			_playing = frames == maxFrames;
			return frames;
		}

	private:
		seir::AudioCallbacks& _callbacks;
		const unsigned _samplingRate;
		seir::Buffer<std::byte, seir::AlignedAllocator<seir::kAudioAlignment>> _buffer;
		std::atomic<bool> _done{ false };
		seir::SharedPtr<seir::AudioSource> _source;
		bool _playing = false;
		bool _started = false;
		bool _stopped = false;
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
