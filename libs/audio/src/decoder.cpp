// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "decoder.hpp"

#include <seir_audio/format.hpp>
#include <seir_io/inlet.hpp>

namespace seir
{
	UniquePtr<AudioDecoder> AudioDecoder::create(SharedPtr<Inlet>&& inlet, [[maybe_unused]] const AudioDecoderPreferences& preferences)
	{
		if (inlet)
			if (const auto id = inlet->get<uint32_t>(0))
				switch (*id)
				{
				case kOggVorbisFileID:
#if SEIR_AUDIO_OGGVORBIS
					return createOggVorbisDecoder(std::move(inlet), preferences);
#else
					break;
#endif
				case kWavFileID:
#if SEIR_AUDIO_WAV
					return createWavDecoder(std::move(inlet), preferences);
#else
					break;
#endif
				default:
#if SEIR_AUDIO_SYNTH
					return createSynthDecoder(inlet, preferences);
#else
					break;
#endif
				}
		return {};
	}
}
