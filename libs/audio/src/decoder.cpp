// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "decoder.hpp"

#include <seir_base/endian.hpp>
#include <seir_data/blob.hpp>

namespace seir
{
	UniquePtr<AudioDecoder> AudioDecoder::create(const SharedPtr<Blob>& blob, [[maybe_unused]] const AudioFormat& preferredFormat)
	{
		if (blob && blob->size() >= sizeof(uint32_t))
			switch (blob->get<uint32_t>(0))
			{
			case makeCC('O', 'g', 'g', 'S'):
#if SEIR_AUDIO_OGGVORBIS
				return createOggVorbisDecoder(blob, preferredFormat);
#else
				break;
#endif
			case makeCC('R', 'I', 'F', 'F'):
#if SEIR_AUDIO_WAV
				return createWavDecoder(blob, preferredFormat);
#else
				break;
#endif
			default:
#if SEIR_AUDIO_AULOS
				return createAulosDecoder(blob, preferredFormat);
#else
				break;
#endif
			}
		return {};
	}
}
