// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "decoder_wav.hpp"

#include <seir_audio/decoder.hpp>
#include <seir_audio/wav.hpp>
#include <seir_data/reader.hpp>

#include <cstring>

namespace
{
	class RawAudioDecoder final : public seir::AudioDecoder
	{
	public:
		RawAudioDecoder(const seir::AudioFormat& format, seir::UniquePtr<seir::Blob>&& blob) noexcept
			: AudioDecoder{ format }
			, _blob{ std::move(blob) }
		{
		}

		size_t decode(void* buffer, size_t maxFrames) override
		{
			// The caller just requires a memory range with subsequent samples,
			// so we could return the pointer to the blob data without copying.
			// However, this is only possible for uncompressed audio, and audio
			// is (almost) never stored uncompressed. This also breaks alignment
			// requirements for processing functions, complicating things further.
			// So, we're sticking to simpler code with extra copying.
			const auto blocks = _reader.readBlocks(maxFrames, _bytesPerFrame);
			std::memcpy(buffer, blocks.first, blocks.second * _bytesPerFrame);
			return blocks.second;
		}

		void restart() override
		{
			_reader.seek(0);
		}

	private:
		const seir::SharedPtr<seir::Blob> _blob;
		seir::Reader _reader{ *_blob };
		const size_t _bytesPerFrame = _format.bytesPerFrame();
	};
}

namespace seir
{
	UniquePtr<AudioDecoder> createWavDecoder(const SharedPtr<Blob>& blob, const AudioFormat&)
	{
		Reader reader{ *blob };
		if (const auto fileHeader = reader.read<WavFileHeader>(); !fileHeader
			|| fileHeader->_riffId != WavFileHeader::RIFF
			|| fileHeader->_waveId != WavFileHeader::WAVE)
			return {};
		const auto findChunk = [&reader](uint32_t id) -> const WavChunkHeader* {
			for (;;)
			{
				const auto header = reader.read<WavChunkHeader>();
				if (!header)
					return nullptr;
				if (header->_id == id)
					return header;
				if (!reader.skip(header->_size))
					return nullptr;
			}
		};
		if (!findChunk(WavChunkHeader::fmt))
			return {};
		const auto fmt = reader.read<WavFormatChunk>();
		if (!fmt)
			return {};
		AudioSampleType sampleType;
		if (fmt->format == WAVE_FORMAT_PCM && fmt->bitsPerSample == 16)
			sampleType = AudioSampleType::i16;
		else if (fmt->format == WAVE_FORMAT_IEEE_FLOAT && fmt->bitsPerSample == 32)
			sampleType = AudioSampleType::f32;
		else
			return {};
		AudioChannelLayout channelLayout;
		switch (fmt->channels)
		{
		case 1: channelLayout = AudioChannelLayout::Mono; break;
		case 2: channelLayout = AudioChannelLayout::Stereo; break;
		default: return {};
		}
		if (fmt->samplesPerSecond < 8000 || fmt->samplesPerSecond > 48000)
			return {};
		const auto dataHeader = findChunk(WavChunkHeader::data);
		if (!dataHeader)
			return {};
		auto dataSize = reader.size() - reader.offset();
		if (dataSize > dataHeader->_size)
			dataSize = dataHeader->_size;
		return makeUnique<AudioDecoder, RawAudioDecoder>(AudioFormat{ sampleType, channelLayout, fmt->samplesPerSecond }, Blob::from(blob, reader.offset(), dataSize));
	}
}
