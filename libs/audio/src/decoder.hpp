// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_audio/decoder.hpp>

#include <seir_base/endian.hpp>

namespace seir
{
#if SEIR_AUDIO_AULOS
	UniquePtr<AudioDecoder> createAulosDecoder(SharedPtr<Blob>&&, const AudioDecoderPreferences&);
#endif
#if SEIR_AUDIO_OGGVORBIS
	constexpr uint32_t kOggVorbisFileID = seir::makeCC('O', 'g', 'g', 'S');
	UniquePtr<AudioDecoder> createOggVorbisDecoder(SharedPtr<Blob>&&, const AudioDecoderPreferences&);
#endif
#if SEIR_AUDIO_WAV
	constexpr uint32_t kWavFileID = seir::makeCC('R', 'I', 'F', 'F');
	UniquePtr<AudioDecoder> createWavDecoder(SharedPtr<Blob>&&, const AudioDecoderPreferences&);
#endif
}
