// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "decoder.hpp"

#include <seir_audio/format.hpp>
#include <seir_data/reader.hpp>

#include <aulos/composition.hpp>
#include <aulos/format.hpp>
#include <aulos/renderer.hpp>

namespace
{
	constexpr aulos::AudioFormat convertFormat(const seir::AudioFormat& format) noexcept
	{
		auto samplingRate = format.samplingRate();
		if (samplingRate < aulos::Renderer::kMinSamplingRate)
			samplingRate = aulos::Renderer::kMinSamplingRate;
		else if (samplingRate > aulos::Renderer::kMaxSamplingRate)
			samplingRate = aulos::Renderer::kMaxSamplingRate;
		auto channels = aulos::ChannelLayout::Stereo;
		switch (format.channelLayout())
		{
		case seir::AudioChannelLayout::Mono: channels = aulos::ChannelLayout::Mono; break;
		case seir::AudioChannelLayout::Stereo: break;
		}
		return { samplingRate, channels };
	}

	constexpr seir::AudioFormat convertFormat(const aulos::AudioFormat& format) noexcept
	{
		auto channels = seir::AudioChannelLayout::Mono;
		switch (format.channelLayout())
		{
		case aulos::ChannelLayout::Mono: break;
		case aulos::ChannelLayout::Stereo: channels = seir::AudioChannelLayout::Stereo; break;
		}
		return { seir::AudioSampleType::f32, channels, format.samplingRate() };
	}

	class AulosAudioDecoder final : public seir::AudioDecoder
	{
	public:
		AulosAudioDecoder(std::unique_ptr<const aulos::Composition>&& composition, std::unique_ptr<aulos::Renderer>&& renderer) noexcept
			: _composition{ std::move(composition) }, _renderer{ std::move(renderer) } {}

		seir::AudioFormat format() const noexcept override
		{
			return _format;
		}

		size_t read(void* buffer, size_t maxFrames) override
		{
			return _renderer->render(static_cast<float*>(buffer), maxFrames);
		}

		bool seek(size_t frameOffset) override
		{
			_renderer->restart();
			_renderer->skipFrames(frameOffset);
			return true;
		}

	private:
		const std::unique_ptr<const aulos::Composition> _composition;
		const std::unique_ptr<aulos::Renderer> _renderer;
		const seir::AudioFormat _format = ::convertFormat(_renderer->format());
	};
}

namespace seir
{
	UniquePtr<AudioDecoder> createAulosDecoder(SharedPtr<Blob>&& blob, const AudioDecoderPreferences& preferences)
	{
		const std::string buffer{ static_cast<const char*>(blob->data()), blob->size() }; // TODO: Remove when Aulos will support non-null-terminated input.
		if (auto composition = aulos::Composition::create(buffer.c_str()))
			if (auto renderer = aulos::Renderer::create(*composition, ::convertFormat(preferences.format), preferences.loop))
				return makeUnique<AudioDecoder, AulosAudioDecoder>(std::move(composition), std::move(renderer));
		return {};
	}
}
