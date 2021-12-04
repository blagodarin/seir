// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "decoder.hpp"

#include <seir_audio/format.hpp>
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

	UniquePtr<AudioDecoder> AudioDecoder::custom(const SharedPtr<AudioDecoder>& decoder)
	{
		class AudioDecoderWrapper final : public AudioDecoderBase
		{
		public:
			AudioDecoderWrapper(const SharedPtr<AudioDecoder>& decoder) noexcept
				: _decoder{ decoder } {}

			bool finished() const noexcept override
			{
				return _finished;
			}

			AudioFormat format() const noexcept override
			{
				return _decoder->format();
			}

			size_t read(void* buffer, size_t maxFrames) override
			{
				const auto result = _decoder->read(buffer, maxFrames);
				if (!_finished && result < maxFrames)
					_finished = true;
				return result;
			}

			bool seek(size_t frameOffset) override
			{
				if (!_decoder->seek(frameOffset))
					return false;
				_finished = false;
				return true;
			}

		private:
			const SharedPtr<AudioDecoder> _decoder;
			bool _finished = false;
		};
		return makeUnique<AudioDecoder, AudioDecoderWrapper>(decoder);
	}
}
