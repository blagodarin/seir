// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "decoder.hpp"

#include <seir_audio/decoder.hpp>
#include <seir_audio/format.hpp>
#include <seir_base/endian.hpp>
#include <seir_data/reader.hpp>

#include <cstring>

namespace
{
	enum : uint16_t
	{
		WAVE_FORMAT_PCM = 0x0001,
		WAVE_FORMAT_IEEE_FLOAT = 0x0003,
	};

#pragma pack(push, 1)

	struct WavFileHeader
	{
		enum : uint32_t
		{
			RIFF = seir::makeCC('R', 'I', 'F', 'F'),
			WAVE = seir::makeCC('W', 'A', 'V', 'E'),
		};

		uint32_t _riffId;
		uint32_t _riffSize;
		uint32_t _waveId;
	};

	struct WavChunkHeader
	{
		enum : uint32_t
		{
			fmt = seir::makeCC('f', 'm', 't', ' '),
			data = seir::makeCC('d', 'a', 't', 'a'),
		};

		uint32_t _id;
		uint32_t _size;
	};

	struct WavFormatChunk
	{
		uint16_t format;
		uint16_t channels;
		uint32_t samplesPerSecond;
		uint32_t bytesPerSecond;
		uint16_t blockAlign;
		uint16_t bitsPerSample;
	};

#pragma pack(pop)

	class RawAudioDecoder final : public seir::AudioDecoder
	{
	public:
		RawAudioDecoder(seir::UniquePtr<seir::Blob>&& blob, const seir::AudioFormat& format) noexcept
			: _blob{ std::move(blob) }
			, _format{ format }
		{
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
	UniquePtr<AudioDecoder> createWavDecoder(const SharedPtr<Blob>& blob, const AudioDecoder::Preferences&)
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
