// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "mixer.hpp"

#include <seir_audio/format.hpp>
#include "decoder.hpp"

#include <cassert>
#include <numeric>

namespace
{
	constexpr size_t kResamplingFractionBits = 16;
	constexpr size_t kResamplingScale = 1 << kResamplingFractionBits;
	constexpr size_t kResamplingMask = kResamplingScale - 1;
	constexpr auto kResamplingPrefixFrames = std::lcm(sizeof(seir::AudioFrame), seir::kAudioAlignment) / sizeof(seir::AudioFrame);
}

namespace seir
{
	void AudioMixer::reset(unsigned samplingRate, size_t maxBufferFrames)
	{
		assert(samplingRate > 0);
		assert(maxBufferFrames > 0);
		_samplingRate = samplingRate;
		_processingBuffer.reserve(maxBufferFrames * sizeof(AudioFrame), false); // Enough for all supported audio frame format.
		_resamplingBuffer.reserve(kResamplingPrefixFrames + (maxBufferFrames * AudioFormat::kMaxSamplingRate + samplingRate - 1) / samplingRate, false);
	}

	size_t AudioMixer::mix(AudioFrame* output, size_t maxFrames, bool rewrite, AudioDecoderBase& decoder) noexcept
	{
		assert(_samplingRate > 0);
		size_t frames = 0;
		if (const auto samplingRate = decoder.format().samplingRate(); samplingRate != _samplingRate)
		{
			assert(maxFrames > 0);
			const auto step = samplingRate * kResamplingScale / _samplingRate;
			auto input = _resamplingBuffer.data() + kResamplingPrefixFrames;
			auto offset = decoder._resamplingState._offset;
			size_t readyFrames = 0;
			if (offset >= step)
			{
				// The decoded audio is being upsampled and
				// we aren't done with the last decoded frame.
				assert(samplingRate < _samplingRate);
				*--input = decoder._resamplingState._lastFrame;
				++readyFrames;
			}
			const auto maxInputFrames = ((offset + (maxFrames - 1) * step) >> kResamplingFractionBits) + 1; // Index of the first input frame we won't touch.
			const auto inputFrames = readyFrames + process(input + readyFrames, maxInputFrames - readyFrames, true, decoder);
			if (inputFrames > 0) [[likely]]
			{
				auto stepCount = ((inputFrames << kResamplingFractionBits) - offset + step - 1) / step;
				if (stepCount > maxFrames)
				{
					// This may happen if the audio is being upsampled and
					// the last input step spans more than one output frame.
					assert(samplingRate < _samplingRate
						&& ((offset + (stepCount - 1) * step) & kResamplingMask) >= step);
					stepCount = maxFrames;
				}
				if (rewrite)
				{
					for (size_t i = 0; i < stepCount; ++i, offset += step)
						output[i] = input[offset >> kResamplingFractionBits];
				}
				else
				{
					for (size_t i = 0; i < stepCount; ++i, offset += step)
						output[i] += input[offset >> kResamplingFractionBits];
				}
				assert((offset - step) >> kResamplingFractionBits == inputFrames - 1);
				frames += stepCount;
				decoder._resamplingState._offset = offset & kResamplingMask;
				decoder._resamplingState._lastFrame = input[inputFrames - 1];
			}
		}
		else
			frames = process(output, maxFrames, rewrite, decoder);
		if (frames > 0 && rewrite)
			std::memset(output + frames, 0, (maxFrames - frames) * sizeof(AudioFrame));
		return frames;
	}

	size_t AudioMixer::process(AudioFrame* output, size_t maxFrames, bool rewrite, AudioDecoderBase& decoder) noexcept
	{
		size_t frames = 0;
		switch (const auto format = decoder.format(); format.channelLayout())
		{
		case AudioChannelLayout::Mono:
			frames = decoder.read(_processingBuffer.data(), maxFrames);
			switch (format.sampleType())
			{
			case AudioSampleType::i16:
				if (rewrite)
					convertSamples2x1D(reinterpret_cast<float*>(output), reinterpret_cast<const int16_t*>(_processingBuffer.data()), frames);
				else
					addSamples2x1D(reinterpret_cast<float*>(output), reinterpret_cast<const int16_t*>(_processingBuffer.data()), frames);
				break;
			case AudioSampleType::f32:
				if (rewrite)
					duplicate1D_32(output, _processingBuffer.data(), frames);
				else
					addSamples2x1D(reinterpret_cast<float*>(output), reinterpret_cast<const float*>(_processingBuffer.data()), frames);
				break;
			}
			break;
		case AudioChannelLayout::Stereo:
			switch (format.sampleType())
			{
			case AudioSampleType::i16:
				frames = decoder.read(_processingBuffer.data(), maxFrames);
				if (rewrite)
					convertSamples1D(reinterpret_cast<float*>(output), reinterpret_cast<const int16_t*>(_processingBuffer.data()), frames * 2);
				else
					addSamples1D(reinterpret_cast<float*>(output), reinterpret_cast<const int16_t*>(_processingBuffer.data()), frames * 2);
				break;
			case AudioSampleType::f32:
				if (rewrite)
					frames = decoder.read(output, maxFrames);
				else
				{
					frames = decoder.read(reinterpret_cast<float*>(_processingBuffer.data()), maxFrames);
					addSamples1D(reinterpret_cast<float*>(output), reinterpret_cast<const float*>(_processingBuffer.data()), frames * 2);
				}
				break;
			}
			break;
		}
		return frames;
	}
}
