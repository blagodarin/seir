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
		size_t _offset = 0;
		AudioFrame _lastFrame{ 0.f, 0.f };
	};

	class AudioDecoderBase : public AudioDecoder
	{
	public:
		AudioResamplingState _resamplingState;

		virtual bool finished() const noexcept = 0;

		void restart() noexcept
		{
			seek(0);
			_resamplingState = {};
		}
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
