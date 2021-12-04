// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "decoder.hpp"

#include <seir_audio/format.hpp>
#include <seir_audio/wav.hpp>
#include <seir_data/reader.hpp>

#include <cstring>

namespace
{
	class RawAudioDecoder final : public seir::AudioDecoderBase
	{
	public:
		RawAudioDecoder(seir::UniquePtr<seir::Blob>&& blob, const seir::AudioFormat& format) noexcept
			: _blob{ std::move(blob) }
			, _format{ format }
		{
		}

		bool finished() const noexcept override
		{
			return _reader.size() - _reader.offset() < _format.bytesPerFrame();
		}

		seir::AudioFormat format() const override
		{
			return _format;
		}

		size_t read(void* buffer, size_t maxFrames) override
		{
			// The caller just requires a memory range with subsequent samples,
			// so we could return the pointer to the blob data without copying.
			// However, this is only possible for uncompressed audio, and audio
			// is (almost) never stored uncompressed. This also breaks alignment
			// requirements for processing functions, complicating things further.
			// So, we're sticking to simpler code with extra copying.
			const auto blocks = _reader.readBlocks(maxFrames, _format.bytesPerFrame());
			std::memcpy(buffer, blocks.first, blocks.second * _format.bytesPerFrame());
			return blocks.second;
		}

		bool seek(size_t frameOffset) override
		{
			return frameOffset <= _reader.size() / _format.bytesPerFrame() // To prevent overflow.
				&& _reader.seek(frameOffset * _format.bytesPerFrame());
		}

	private:
		const seir::SharedPtr<seir::Blob> _blob;
		seir::Reader _reader{ *_blob };
		const seir::AudioFormat _format;
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
		if (fmt->samplesPerSecond < AudioFormat::kMinSamplingRate || fmt->samplesPerSecond > AudioFormat::kMaxSamplingRate)
			return {};
		const auto dataHeader = findChunk(WavChunkHeader::data);
		if (!dataHeader)
			return {};
		auto dataSize = reader.size() - reader.offset();
		if (dataSize > dataHeader->_size)
			dataSize = dataHeader->_size;
		return makeUnique<AudioDecoder, RawAudioDecoder>(Blob::from(blob, reader.offset(), dataSize), AudioFormat{ sampleType, channelLayout, fmt->samplesPerSecond });
	}
}
