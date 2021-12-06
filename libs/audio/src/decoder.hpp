// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_audio/decoder.hpp>

namespace seir
{
#if SEIR_AUDIO_AULOS
	UniquePtr<AudioDecoder> createAulosDecoder(const SharedPtr<Blob>&, const AudioDecoderPreferences&);
#endif
#if SEIR_AUDIO_OGGVORBIS
	UniquePtr<AudioDecoder> createOggVorbisDecoder(const SharedPtr<Blob>&, const AudioDecoderPreferences&);
#endif
#if SEIR_AUDIO_WAV
	UniquePtr<AudioDecoder> createWavDecoder(const SharedPtr<Blob>&, const AudioDecoderPreferences&);
#endif
}
