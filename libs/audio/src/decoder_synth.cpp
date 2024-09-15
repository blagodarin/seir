// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "decoder.hpp"

#include <seir_audio/format.hpp>
#include <seir_io/reader.hpp>
#include <seir_synth/composition.hpp>
#include <seir_synth/format.hpp>
#include <seir_synth/renderer.hpp>

namespace
{
	constexpr seir::synth::AudioFormat convertFormat(const seir::AudioFormat& format) noexcept
	{
		auto samplingRate = format.samplingRate();
		if (samplingRate < seir::synth::Renderer::kMinSamplingRate)
			samplingRate = seir::synth::Renderer::kMinSamplingRate;
		else if (samplingRate > seir::synth::Renderer::kMaxSamplingRate)
			samplingRate = seir::synth::Renderer::kMaxSamplingRate;
		auto channels = seir::synth::ChannelLayout::Stereo;
		switch (format.channelLayout())
		{
		case seir::AudioChannelLayout::Mono: channels = seir::synth::ChannelLayout::Mono; break;
		case seir::AudioChannelLayout::Stereo: break;
		}
		return { samplingRate, channels };
	}

	constexpr seir::AudioFormat convertFormat(const seir::synth::AudioFormat& format) noexcept
	{
		auto channels = seir::AudioChannelLayout::Mono;
		switch (format.channelLayout())
		{
		case seir::synth::ChannelLayout::Mono: break;
		case seir::synth::ChannelLayout::Stereo: channels = seir::AudioChannelLayout::Stereo; break;
		}
		return { seir::AudioSampleType::f32, channels, format.samplingRate() };
	}

	class SynthAudioDecoder final : public seir::AudioDecoder
	{
	public:
		SynthAudioDecoder(std::unique_ptr<const seir::synth::Composition>&& composition, std::unique_ptr<seir::synth::Renderer>&& renderer) noexcept
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
		const std::unique_ptr<const seir::synth::Composition> _composition;
		const std::unique_ptr<seir::synth::Renderer> _renderer;
		const seir::AudioFormat _format = ::convertFormat(_renderer->format());
	};
}

namespace seir
{
	UniquePtr<AudioDecoder> createSynthDecoder(const SharedPtr<Blob>& blob, const AudioDecoderPreferences& preferences)
	{
		const std::string buffer{ static_cast<const char*>(blob->data()), blob->size() }; // TODO: Remove when synth will support non-null-terminated input.
		if (auto composition = synth::Composition::create(buffer.c_str()))
			if (auto renderer = synth::Renderer::create(*composition, ::convertFormat(preferences.format), preferences.loop))
				return makeUnique<AudioDecoder, SynthAudioDecoder>(std::move(composition), std::move(renderer));
		return {};
	}
}
