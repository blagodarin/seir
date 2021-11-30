// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "mixer.hpp"

#include "decoder.hpp"

#include <cassert>

namespace
{
	constexpr int kResamplingFractionBits = 16;
	constexpr int kResamplingScale = 1 << kResamplingFractionBits;
	constexpr int kResamplingMask = kResamplingScale - 1;
}

namespace seir
{
	void AudioMixer::reset(unsigned samplingRate, size_t maxBufferFrames)
	{
		assert(samplingRate > 0);
		assert(maxBufferFrames > 0);
		_samplingRate = samplingRate;
		_processingBuffer.reserve(maxBufferFrames * 2 * sizeof(float), false);
		_resamplingBuffer.reserve(maxBufferFrames, false);
	}

	size_t AudioMixer::mix(AudioFrame* output, size_t maxFrames, bool rewrite, AudioDecoderBase& decoder) noexcept
	{
		assert(_samplingRate > 0);
		size_t frames = 0;
		if (const auto samplingRate = decoder.format().samplingRate(); samplingRate != _samplingRate)
		{
			const auto step = static_cast<int>(samplingRate * kResamplingScale / _samplingRate);
			frames = decoder._resamplingState.flush(output, maxFrames, rewrite, step);
			if (const auto maxOutputFrames = static_cast<int>(maxFrames - frames); maxOutputFrames > 0) [[likely]]
			{
				auto offset = decoder._resamplingState._offset;
				const auto maxInputFrames = ((offset + (maxOutputFrames - 1) * step) >> kResamplingFractionBits) + 1;
				const auto inputFrames = process(_resamplingBuffer.data(), static_cast<size_t>(maxInputFrames), true, decoder);
				if (inputFrames > 0) [[likely]]
				{
					auto stepCount = (((static_cast<int>(inputFrames) << kResamplingFractionBits) - 1) - offset) / step + 1;
					if (stepCount > maxOutputFrames)
					{
						// This may happen if the audio is being upsampled and
						// the last input step spans more than one output frame.
						// TODO: Assert.
						stepCount = maxOutputFrames;
					}
					const auto dst = output + frames;
					const auto src = _resamplingBuffer.data();
					if (rewrite)
					{
						for (int i = 0; i < stepCount; ++i, offset += step)
							dst[i] = src[offset >> kResamplingFractionBits];
					}
					else
					{
						for (int i = 0; i < stepCount; ++i, offset += step)
							dst[i] += src[offset >> kResamplingFractionBits];
					}
					assert((offset - step) >> kResamplingFractionBits == static_cast<int>(inputFrames) - 1);
					frames += static_cast<size_t>(stepCount);
					decoder._resamplingState._offset = offset & kResamplingMask;
					if (decoder._resamplingState._offset >= step)
					{
						decoder._resamplingState._offset -= kResamplingScale;
						decoder._resamplingState._lastFrame = src[inputFrames - 1];
					}
				}
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
