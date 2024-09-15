// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "decoder.hpp"

#include <seir_audio/decoder.hpp>
#include <seir_audio/format.hpp>
#include <seir_base/endian.hpp>
#include <seir_io/reader.hpp>

#include <cstring>

namespace
{
	enum : uint16_t
	{
		WAVE_FORMAT_PCM = 0x0001,
		WAVE_FORMAT_IEEE_FLOAT = 0x0003,
	};

#pragma pack(push, 1)

	struct RiffFileHeader
	{
		uint32_t id;
		uint32_t size;
		uint32_t type;
	};

	struct RiffChunkHeader
	{
		uint32_t id;
		uint32_t size;
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
		RawAudioDecoder(seir::SharedPtr<seir::Blob>&& blob, const seir::AudioFormat& format) noexcept
			: _blob{ std::move(blob) }, _format{ format } {}

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
			const auto [base, count] = _reader.readBlocks(maxFrames, _format.bytesPerFrame());
			std::memcpy(buffer, base, count * _format.bytesPerFrame());
			return count;
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
	UniquePtr<AudioDecoder> createWavDecoder(SharedPtr<Blob>&& blob, const AudioDecoderPreferences&)
	{
		Reader reader{ *blob };
		if (const auto fileHeader = reader.read<RiffFileHeader>(); !fileHeader
			|| fileHeader->id != kWavFileID
			|| fileHeader->type != makeCC('W', 'A', 'V', 'E'))
			return {};
		const auto findChunk = [&reader](uint32_t id) -> const RiffChunkHeader* {
			for (;;)
			{
				const auto header = reader.read<RiffChunkHeader>();
				if (!header)
					return nullptr;
				if (header->id == id)
					return header;
				if (!reader.skip(header->size))
					return nullptr;
			}
		};
		const auto fmtHeader = findChunk(makeCC('f', 'm', 't', ' '));
		if (!fmtHeader || fmtHeader->size < sizeof(WavFormatChunk))
			return {};
		const auto fmt = reader.read<WavFormatChunk>();
		if (!fmt || !reader.skip(fmtHeader->size - sizeof(WavFormatChunk)))
			return {};
		AudioSampleType sampleType; // NOLINT(cppcoreguidelines-init-variables)
		if (fmt->format == WAVE_FORMAT_PCM && fmt->bitsPerSample == 16)
			sampleType = AudioSampleType::i16;
		else if (fmt->format == WAVE_FORMAT_IEEE_FLOAT && fmt->bitsPerSample == 32)
			sampleType = AudioSampleType::f32;
		else
			return {};
		AudioChannelLayout channelLayout; // NOLINT(cppcoreguidelines-init-variables)
		switch (fmt->channels)
		{
		case 1: channelLayout = AudioChannelLayout::Mono; break;
		case 2: channelLayout = AudioChannelLayout::Stereo; break;
		default: return {};
		}
		if (fmt->samplesPerSecond < AudioFormat::kMinSamplingRate || fmt->samplesPerSecond > AudioFormat::kMaxSamplingRate)
			return {};
		const auto dataHeader = findChunk(makeCC('d', 'a', 't', 'a'));
		if (!dataHeader)
			return {};
		auto dataSize = reader.size() - reader.offset();
		if (dataSize > dataHeader->size)
			dataSize = dataHeader->size;
		return makeUnique<AudioDecoder, RawAudioDecoder>(Blob::from(std::move(blob), reader.offset(), dataSize), AudioFormat{ sampleType, channelLayout, fmt->samplesPerSecond });
	}
}
