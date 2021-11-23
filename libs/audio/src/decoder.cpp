// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_audio/decoder.hpp>

#include <seir_base/endian.hpp>
#include <seir_data/blob.hpp>

#if SEIR_AUDIO_WAV
#	include "decoder_wav.hpp"
#endif

namespace seir
{
	UniquePtr<AudioDecoder> AudioDecoder::create(const SharedPtr<Blob>& blob, [[maybe_unused]] const AudioFormat& preferredFormat)
	{
		if (blob && blob->size() >= sizeof(uint32_t))

			switch (blob->get<uint32_t>(0))
			{
			case makeCC('R', 'I', 'F', 'F'):
#if SEIR_AUDIO_WAV
				return createWavDecoder(blob, preferredFormat);
#else
				break;
#endif
			default:
				break;
			}
		return {};
	}
}
