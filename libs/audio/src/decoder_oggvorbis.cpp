// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "decoder.hpp"

#include <seir_audio/decoder.hpp>
#include <seir_audio/format.hpp>
#include <seir_data/reader.hpp>
#include <seir_base/int_utils.hpp>

#include <cstring>
#include <limits>

#define OV_EXCLUDE_STATIC_CALLBACKS
#include <vorbis/vorbisfile.h>

namespace
{
	class OggVorbisAudioDecoder final : public seir::AudioDecoder
	{
	public:
		OggVorbisAudioDecoder(const seir::SharedPtr<seir::Blob>& blob) noexcept
			: _blob{ blob }
		{
		}

		~OggVorbisAudioDecoder() noexcept override
		{
			::ov_clear(&_oggVorbis);
		}

		bool open() noexcept
		{
			if (::ov_open_callbacks(&_reader, &_oggVorbis, nullptr, 0, { readCallback, seekCallback, closeCallback, tellCallback }) < 0)
				return false;
			vorbis_info* info = ::ov_info(&_oggVorbis, -1);
			seir::AudioChannelLayout channelLayout;
			switch (info->channels)
			{
			case 1: channelLayout = seir::AudioChannelLayout::Mono; break;
			case 2: channelLayout = seir::AudioChannelLayout::Stereo; break;
			default: return false;
			}
			if (info->rate < seir::AudioFormat::kMinSamplingRate || info->rate > seir::AudioFormat::kMaxSamplingRate)
				return false;
			const auto totalFrames = ::ov_pcm_total(&_oggVorbis, -1);
			if (totalFrames < 0)
				return false;
			_format = { seir::AudioSampleType::f32, channelLayout, static_cast<unsigned>(info->rate) };
			_totalFrames = static_cast<size_t>(totalFrames);
			return true;
		}

		seir::AudioFormat format() const override
		{
			return _format;
		}

		size_t read(void* buffer, size_t maxFrames) override
		{
			const auto framesToRead = std::min(maxFrames, _totalFrames - _currentFrame);
			size_t framesRead = 0;
			while (framesRead < framesToRead)
			{
				float** src = nullptr;
				const auto srcFrames = ::ov_read_float(&_oggVorbis, &src, static_cast<int>(std::min(framesToRead - framesRead, size_t{ std::numeric_limits<int>::max() })), nullptr);
				if (srcFrames <= 0)
					break;
				const auto dst = static_cast<float*>(buffer) + framesRead * _format.channels();
				for (auto i = decltype(srcFrames){}; i < srcFrames; ++i)
				{
					dst[2 * i] = src[0][i];
					dst[2 * i + 1] = src[1][i];
				}
				framesRead += seir::toUnsigned(srcFrames);
			}
			_currentFrame += framesRead;
			return framesRead;
		}

		bool seek(size_t frameOffset) override
		{
			if (frameOffset > _totalFrames
				|| ::ov_pcm_seek(&_oggVorbis, static_cast<ogg_int64_t>(frameOffset)) != 0)
				return false;
			_currentFrame = frameOffset;
			return true;
		}

	private:
		static size_t readCallback(void* ptr, size_t size, size_t nmemb, void* datasource)
		{
			const auto blocks = static_cast<seir::Reader*>(datasource)->readBlocks(nmemb, size);
			std::memcpy(ptr, blocks.first, blocks.second * size);
			return blocks.second;
		}

		static int seekCallback(void* datasource, ogg_int64_t offset, int whence)
		{
			auto& reader = *static_cast<seir::Reader*>(datasource);
			switch (whence)
			{
			case SEEK_SET: return offset >= 0 && reader.seek(static_cast<size_t>(offset)) ? 0 : -1;
			case SEEK_CUR: return offset >= 0 && reader.skip(static_cast<size_t>(offset)) ? 0 : -1;
			case SEEK_END: return offset == 0 && reader.seek(reader.size()) ? 0 : -1;
			default: return -1;
			}
		}

		static int closeCallback(void*)
		{
			return 0;
		}

		static long tellCallback(void* datasource)
		{
			return static_cast<long>(static_cast<seir::Reader*>(datasource)->offset());
		}

	private:
		const seir::SharedPtr<seir::Blob> _blob;
		seir::Reader _reader{ *_blob };
		OggVorbis_File _oggVorbis{};
		seir::AudioFormat _format;
		size_t _totalFrames = 0;
		size_t _currentFrame = 0;
	};
}

namespace seir
{
	UniquePtr<AudioDecoder> createOggVorbisDecoder(const SharedPtr<Blob>& blob, const AudioFormat&)
	{
		auto decoder = makeUnique<OggVorbisAudioDecoder>(blob);
		return UniquePtr<AudioDecoder>{ decoder->open() ? std::move(decoder) : nullptr };
	}
}
