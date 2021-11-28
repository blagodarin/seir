// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_audio/player.hpp>

#include "backend.hpp"

#include <seir_audio/decoder.hpp>
#include <seir_base/buffer.hpp>

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

		void play(const seir::SharedPtr<seir::AudioDecoder>& decoder) override
		{
			assert(decoder);
			if (decoder->format().samplingRate() != _samplingRate)
				return;
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
			{
				size_t frames = 0;
				switch (const auto format = decoder->format(); format.channelLayout())
				{
				case seir::AudioChannelLayout::Mono:
					frames = decoder->read(_buffer.data(), maxFrames);
					switch (format.sampleType())
					{
					case seir::AudioSampleType::i16:
						if (!mixedFrames)
							seir::convertSamples2x1D(output, reinterpret_cast<const int16_t*>(_buffer.data()), frames);
						else
							seir::addSamples2x1D(output, reinterpret_cast<const int16_t*>(_buffer.data()), frames);
						break;
					case seir::AudioSampleType::f32:
						if (!mixedFrames)
							seir::duplicate1D_32(output, _buffer.data(), frames);
						else
							seir::addSamples2x1D(output, reinterpret_cast<const float*>(_buffer.data()), frames);
						break;
					}
					break;
				case seir::AudioChannelLayout::Stereo:
					switch (format.sampleType())
					{
					case seir::AudioSampleType::i16:
						frames = decoder->read(_buffer.data(), maxFrames);
						if (!mixedFrames)
							seir::convertSamples1D(output, reinterpret_cast<const int16_t*>(_buffer.data()), frames * 2);
						else
							seir::addSamples1D(output, reinterpret_cast<const int16_t*>(_buffer.data()), frames * 2);
						break;
					case seir::AudioSampleType::f32:
						frames = decoder->read(mixedFrames ? reinterpret_cast<float*>(_buffer.data()) : output, maxFrames);
						break;
					}
					break;
				}
				if (frames > mixedFrames)
				{
					if (!mixedFrames)
						std::memset(output + frames * 2, 0, (maxFrames - frames) * 2);
					mixedFrames = frames;
				}
			}
			return mixedFrames;
		}

	private:
		seir::AudioCallbacks& _callbacks;
		const unsigned _samplingRate;
		seir::Buffer<std::byte, seir::AlignedAllocator<seir::kAudioAlignment>> _buffer;
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
