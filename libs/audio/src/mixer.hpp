// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_audio/processing.hpp>
#include <seir_base/buffer.hpp>
#include "frame.hpp"

namespace seir
{
	class AudioDecoderBase;

	class AudioMixer
	{
	public:
		void reset(unsigned samplingRate, size_t maxBufferFrames);
		size_t mix(AudioFrame* output, size_t maxFrames, bool rewrite, AudioDecoderBase&) noexcept;

	private:
		size_t process(AudioFrame* output, size_t maxFrames, bool rewrite, AudioDecoderBase&) noexcept;

	private:
		unsigned _samplingRate = 0;
		Buffer<std::byte, AlignedAllocator<kAudioAlignment>> _processingBuffer;
		Buffer<AudioFrame, AlignedAllocator<kAudioAlignment>> _resamplingBuffer;
	};
}
