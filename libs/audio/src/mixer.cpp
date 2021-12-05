// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "mixer.hpp"

#include <seir_audio/decoder.hpp>
#include "processing.hpp"

#include <cassert>
#include <numeric>

namespace seir
{
	void AudioMixer::reset(unsigned samplingRate, size_t maxBufferFrames)
	{
		assert(samplingRate > 0);
		assert(maxBufferFrames > 0);
		_samplingRate = samplingRate;
		_processingBuffer.reserve(maxBufferFrames * kAudioFrameSize, false); // Enough for all supported audio frame format.
		_resamplingBuffer.reserve((kAudioFramesPerBlock + (maxBufferFrames * AudioFormat::kMaxSamplingRate + samplingRate - 1) / samplingRate) * kAudioChannels, false);
	}

	size_t AudioMixer::mix(float* output, size_t maxFrames, bool rewrite, AudioDecoder& decoder) noexcept
	{
		assert(_samplingRate > 0);
		size_t frames = 0;
		if (const auto samplingRate = decoder.format().samplingRate(); samplingRate != _samplingRate)
		{
			assert(maxFrames > 0);
			const auto step = (samplingRate << kAudioResamplingFractionBits) / _samplingRate;
			auto input = _resamplingBuffer.data() + kAudioFramesPerBlock;
			auto offset = decoder._internal._resamplingOffset;
			size_t readyFrames = 0;
			if (offset >= step)
			{
				// The decoded audio is being upsampled and
				// we aren't done with the last decoded frame.
				assert(samplingRate < _samplingRate);
				input -= kAudioChannels;
				++readyFrames;
				static_assert(sizeof decoder._internal._resamplingBuffer == kAudioFrameSize);
				std::memcpy(input, decoder._internal._resamplingBuffer, kAudioFrameSize);
			}
			const auto maxInputFrames = ((offset + (maxFrames - 1) * step) >> kAudioResamplingFractionBits) + 1; // Index of the first input frame we won't touch.
			const auto inputFrames = readyFrames + process(input + readyFrames, maxInputFrames - readyFrames, true, decoder);
			if (inputFrames > 0) [[likely]]
			{
				auto stepCount = ((inputFrames << kAudioResamplingFractionBits) - offset + step - 1) / step;
				if (stepCount > maxFrames)
				{
					// This may happen if the audio is being upsampled and
					// the last input step spans more than one output frame.
					assert(samplingRate < _samplingRate
						&& ((offset + (stepCount - 1) * step) & kAudioResamplingFractionMask) >= step);
					stepCount = maxFrames;
				}
				assert((offset + (stepCount - 1) * step) >> kAudioResamplingFractionBits == inputFrames - 1);
				if (rewrite)
					resampleCopy2x1D(output, stepCount, input, offset, step);
				else
					resampleAdd2x1D(output, stepCount, input, offset, step);
				frames += stepCount;
				decoder._internal._resamplingOffset = offset & kAudioResamplingFractionMask;
				static_assert(sizeof decoder._internal._resamplingBuffer == kAudioFrameSize);
				std::memcpy(decoder._internal._resamplingBuffer, input + (inputFrames - 1) * kAudioChannels, kAudioFrameSize);
			}
		}
		else
			frames = process(output, maxFrames, rewrite, decoder);
		if (frames < maxFrames)
		{
			decoder._internal._finished = true;
			if (frames > 0 && rewrite)
				std::memset(output + frames * kAudioChannels, 0, (maxFrames - frames) * kAudioFrameSize);
		}
		return frames;
	}

	size_t AudioMixer::process(float* output, size_t maxFrames, bool rewrite, AudioDecoder& decoder) noexcept
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
					convertSamples2x1D(output, reinterpret_cast<const int16_t*>(_processingBuffer.data()), frames);
				else
					addSamples2x1D(output, reinterpret_cast<const int16_t*>(_processingBuffer.data()), frames);
				break;
			case AudioSampleType::f32:
				if (rewrite)
					duplicate1D_32(output, _processingBuffer.data(), frames);
				else
					addSamples2x1D(output, reinterpret_cast<const float*>(_processingBuffer.data()), frames);
				break;
			}
			break;
		case AudioChannelLayout::Stereo:
			switch (format.sampleType())
			{
			case AudioSampleType::i16:
				frames = decoder.read(_processingBuffer.data(), maxFrames);
				if (rewrite)
					convertSamples1D(output, reinterpret_cast<const int16_t*>(_processingBuffer.data()), frames * kAudioChannels);
				else
					addSamples1D(output, reinterpret_cast<const int16_t*>(_processingBuffer.data()), frames * kAudioChannels);
				break;
			case AudioSampleType::f32:
				if (rewrite)
					frames = decoder.read(output, maxFrames);
				else
				{
					frames = decoder.read(reinterpret_cast<float*>(_processingBuffer.data()), maxFrames);
					addSamples1D(output, reinterpret_cast<const float*>(_processingBuffer.data()), frames * kAudioChannels);
				}
				break;
			}
			break;
		}
		return frames;
	}
}
