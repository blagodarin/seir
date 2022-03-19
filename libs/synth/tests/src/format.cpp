// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_synth/format.hpp>

#include <doctest/doctest.h>

constexpr seir::synth::AudioFormat kMono44KHz{ 44'100, seir::synth::ChannelLayout::Mono };
constexpr seir::synth::AudioFormat kStereo48KHz{ 48'000, seir::synth::ChannelLayout::Stereo };

TEST_CASE("format_construction")
{
	CHECK(kMono44KHz.bytesPerFrame() == 4);
	CHECK(kMono44KHz.channelCount() == 1);
	CHECK(kMono44KHz.channelLayout() == seir::synth::ChannelLayout::Mono);
	CHECK(kMono44KHz.samplingRate() == 44'100);

	CHECK(kStereo48KHz.bytesPerFrame() == 8);
	CHECK(kStereo48KHz.channelCount() == 2);
	CHECK(kStereo48KHz.channelLayout() == seir::synth::ChannelLayout::Stereo);
	CHECK(kStereo48KHz.samplingRate() == 48'000);
}

TEST_CASE("format_comparison")
{
	CHECK(kMono44KHz == seir::synth::AudioFormat(44'100, seir::synth::ChannelLayout::Mono));
	CHECK(kMono44KHz != seir::synth::AudioFormat(44'100, seir::synth::ChannelLayout::Stereo));
	CHECK(kMono44KHz != seir::synth::AudioFormat(48'000, seir::synth::ChannelLayout::Mono));
	CHECK(kMono44KHz != seir::synth::AudioFormat(48'000, seir::synth::ChannelLayout::Stereo));

	CHECK(kStereo48KHz != seir::synth::AudioFormat(44'100, seir::synth::ChannelLayout::Mono));
	CHECK(kStereo48KHz != seir::synth::AudioFormat(44'100, seir::synth::ChannelLayout::Stereo));
	CHECK(kStereo48KHz != seir::synth::AudioFormat(48'000, seir::synth::ChannelLayout::Mono));
	CHECK(kStereo48KHz == seir::synth::AudioFormat(48'000, seir::synth::ChannelLayout::Stereo));
}
