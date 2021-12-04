// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_audio/processing.hpp>
#include <seir_base/buffer.hpp>

namespace seir
{
	class AudioDecoderBase;

	class AudioMixer
	{
	public:
		void reset(unsigned samplingRate, size_t maxBufferFrames);
		size_t mix(float* output, size_t maxFrames, bool rewrite, AudioDecoderBase&) noexcept;

	private:
		size_t process(float* output, size_t maxFrames, bool rewrite, AudioDecoderBase&) noexcept;

	private:
		unsigned _samplingRate = 0;
		Buffer<std::byte, AlignedAllocator<kAudioBlockAlignment>> _processingBuffer;
		Buffer<float, AlignedAllocator<kAudioBlockAlignment>> _resamplingBuffer;
	};
}
