// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "decoder.hpp"

#include <seir_audio/format.hpp>
#include <seir_io/blob.hpp>

namespace seir
{
	UniquePtr<AudioDecoder> AudioDecoder::create(SharedPtr<Blob>&& blob, [[maybe_unused]] const AudioDecoderPreferences& preferences)
	{
		if (blob)
			if (const auto id = blob->get<uint32_t>(0))
				switch (*id)
				{
				case kOggVorbisFileID:
#if SEIR_AUDIO_OGGVORBIS
					return createOggVorbisDecoder(std::move(blob), preferences);
#else
					break;
#endif
				case kWavFileID:
#if SEIR_AUDIO_WAV
					return createWavDecoder(std::move(blob), preferences);
#else
					break;
#endif
				default:
#if SEIR_AUDIO_SYNTH
					return createSynthDecoder(blob, preferences);
#else
					break;
#endif
				}
		return {};
	}
}
