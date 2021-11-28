// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "mixer.hpp"

#include <seir_audio/decoder.hpp>

#include <cassert>

namespace seir
{
	void AudioMixer::reset(unsigned samplingRate, size_t maxBufferFrames)
	{
		assert(samplingRate > 0);
		assert(maxBufferFrames > 0);
		_samplingRate = samplingRate;
		_processingBuffer.reserve(maxBufferFrames * 2 * sizeof(float), false);
		_resamplingBuffer.reserve(maxBufferFrames * 2, false);
	}

	size_t AudioMixer::mix(float* output, size_t maxFrames, bool rewrite, AudioDecoder& decoder) noexcept
	{
		assert(_samplingRate > 0);
		size_t frames = 0;
		if (const auto samplingRate = decoder.format().samplingRate(); samplingRate != _samplingRate)
		{
			const auto ratio = samplingRate * (1u << 16) / _samplingRate;
			const auto processedFrames = process(_resamplingBuffer.data(), maxFrames * samplingRate / _samplingRate, true, decoder);
			frames = processedFrames * _samplingRate / samplingRate;
			if (rewrite)
			{
				for (size_t i = 0, j = 0; i < frames; ++i, j += ratio)
				{
					output[i * 2] = _resamplingBuffer.data()[(j >> 16) * 2];
					output[i * 2 + 1] = _resamplingBuffer.data()[(j >> 16) * 2 + 1];
				}
			}
			else
			{
				for (size_t i = 0, j = 0; i < frames; ++i, j += ratio)
				{
					output[i * 2] += _resamplingBuffer.data()[(j >> 16) * 2];
					output[i * 2 + 1] += _resamplingBuffer.data()[(j >> 16) * 2 + 1];
				}
			}
		}
		else
			frames = process(output, maxFrames, rewrite, decoder);
		if (frames > 0 && rewrite)
			std::memset(output + frames * 2, 0, (maxFrames - frames) * 2);
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
					convertSamples1D(output, reinterpret_cast<const int16_t*>(_processingBuffer.data()), frames * 2);
				else
					addSamples1D(output, reinterpret_cast<const int16_t*>(_processingBuffer.data()), frames * 2);
				break;
			case AudioSampleType::f32:
				if (rewrite)
					frames = decoder.read(output, maxFrames);
				else
				{
					frames = decoder.read(reinterpret_cast<float*>(_processingBuffer.data()), maxFrames);
					addSamples1D(output, reinterpret_cast<const float*>(_processingBuffer.data()), frames * 2);
				}
				break;
			}
			break;
		}
		return frames;
	}
}
