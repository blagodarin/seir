// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

namespace seir
{
	enum class AudioSampleType
	{
		i16 = 2,
		f32 = 4,
	};

	enum class AudioChannelLayout
	{
		Mono = 1,   // One channel.
		Stereo = 2, // Two channels (interleaved left-right).
	};

	class AudioFormat
	{
	public:
		constexpr AudioFormat() noexcept = default;
		constexpr AudioFormat(AudioSampleType sampleType, AudioChannelLayout channelLayout, unsigned samplingRate) noexcept
			: _sampleType{ sampleType }, _channelLayout{ channelLayout }, _samplingRate{ samplingRate } {}

		[[nodiscard]] constexpr unsigned bytesPerFrame() const noexcept { return bytesPerSample() * channels(); }
		[[nodiscard]] constexpr unsigned bytesPerSample() const noexcept { return static_cast<unsigned>(_sampleType); }
		[[nodiscard]] constexpr unsigned bytesPerSecond() const noexcept { return bytesPerFrame() * _samplingRate; }
		[[nodiscard]] constexpr unsigned channels() const noexcept { return static_cast<unsigned>(_channelLayout); }
		[[nodiscard]] constexpr unsigned samplingRate() const noexcept { return _samplingRate; }
		[[nodiscard]] constexpr AudioSampleType sampleType() const noexcept { return _sampleType; }

	private:
		AudioSampleType _sampleType = AudioSampleType::i16;
		AudioChannelLayout _channelLayout = AudioChannelLayout::Mono;
		unsigned _samplingRate = 0;
	};
}
