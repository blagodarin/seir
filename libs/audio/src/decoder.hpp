// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_audio/decoder.hpp>

#include <seir_base/endian.hpp>

namespace seir
{
	constexpr uint32_t kOggVorbisFileID = seir::makeCC('O', 'g', 'g', 'S');
#if SEIR_AUDIO_OGGVORBIS
	UniquePtr<AudioDecoder> createOggVorbisDecoder(SharedPtr<Blob>&&, const AudioDecoderPreferences&);
#endif

#if SEIR_AUDIO_SYNTH
	UniquePtr<AudioDecoder> createSynthDecoder(SharedPtr<Blob>&&, const AudioDecoderPreferences&);
#endif

	constexpr uint32_t kWavFileID = seir::makeCC('R', 'I', 'F', 'F');
#if SEIR_AUDIO_WAV
	UniquePtr<AudioDecoder> createWavDecoder(SharedPtr<Blob>&&, const AudioDecoderPreferences&);
#endif
}
