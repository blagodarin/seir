// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_audio/decoder.hpp>
#include <seir_data/blob.hpp>
#include <seir_data/file.hpp>

#include <doctest/doctest.h>

TEST_CASE("AudioDecoder")
{
#if SEIR_AUDIO_WAV
	SUBCASE("44100_mono_f32.wav")
	{
		auto blob = seir::createFileBlob(SEIR_TEST_DIR "44100_mono_f32.wav");
		REQUIRE(blob);
		const auto decoder = seir::AudioDecoder::create(seir::SharedPtr{ std::move(blob) });
		REQUIRE(decoder);
		SUBCASE("read()")
		{
			const auto format = decoder->format();
			REQUIRE(format.channelLayout() == seir::AudioChannelLayout::Mono);
			REQUIRE(format.sampleType() == seir::AudioSampleType::f32);
			CHECK(format.samplingRate() == 44'100);
			float samples[1]{};
			CHECK(decoder->read(samples, 1) == 0);
		}
		SUBCASE("seek(0)")
		{
			CHECK(decoder->seek(0));
		}
		SUBCASE("seek(1)")
		{
			CHECK_FALSE(decoder->seek(1));
		}
	}
	SUBCASE("48000_stereo_f32.wav")
	{
		auto blob = seir::createFileBlob(SEIR_TEST_DIR "48000_stereo_f32.wav");
		REQUIRE(blob);
		const auto decoder = seir::AudioDecoder::create(seir::SharedPtr{ std::move(blob) });
		REQUIRE(decoder);
		SUBCASE("read()")
		{
			const auto format = decoder->format();
			REQUIRE(format.channelLayout() == seir::AudioChannelLayout::Stereo);
			REQUIRE(format.sampleType() == seir::AudioSampleType::f32);
			CHECK(format.samplingRate() == 48'000);
			float samples[2]{};
			CHECK(decoder->read(samples, 1) == 0);
		}
		SUBCASE("seek(0)")
		{
			CHECK(decoder->seek(0));
		}
		SUBCASE("seek(1)")
		{
			CHECK_FALSE(decoder->seek(1));
		}
	}
#endif
}