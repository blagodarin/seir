// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_audio/decoder.hpp>

#include "frame.hpp"

namespace seir
{
	struct AudioResamplingState
	{
		int _offset = 0;
		AudioFrame _lastFrame{ 0.f, 0.f };

		constexpr size_t flush(AudioFrame* output, size_t maxFrames, bool rewrite, int step) noexcept
		{
			size_t i = 0;
			for (; _offset < 0 && i < maxFrames; ++i, _offset += step)
				if (rewrite)
					output[i] = _lastFrame;
				else
					output[i] += _lastFrame;
			return i;
		}
	};

	class AudioDecoderBase : public AudioDecoder
	{
	public:
		AudioResamplingState _resamplingState;

		bool finished() const noexcept
		{
			return finishedDecoding() && _resamplingState._offset >= 0;
		}

		void restart() noexcept
		{
			seek(0);
			_resamplingState = {};
		}

	protected:
		virtual bool finishedDecoding() const noexcept = 0;
	};

#if SEIR_AUDIO_AULOS
	UniquePtr<AudioDecoder> createAulosDecoder(const SharedPtr<Blob>&, const AudioFormat&);
#endif
#if SEIR_AUDIO_OGGVORBIS
	UniquePtr<AudioDecoder> createOggVorbisDecoder(const SharedPtr<Blob>&, const AudioFormat&);
#endif
#if SEIR_AUDIO_WAV
	UniquePtr<AudioDecoder> createWavDecoder(const SharedPtr<Blob>&, const AudioFormat&);
#endif
}
