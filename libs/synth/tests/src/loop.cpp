// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_synth/composition.hpp>
#include <seir_synth/data.hpp>
#include <seir_synth/format.hpp>
#include <seir_synth/renderer.hpp>

#include <array>
#include <functional>

#include <doctest/doctest.h>

using namespace std::chrono_literals;

namespace
{
	constexpr auto kTestSamplingRate = 8'800u;
	constexpr auto kTestNote = seir::synth::Note::A4; // 440 Hz.
	constexpr auto kTestWavePeriod = 20u;
	constexpr auto kTestSamples = (kTestSamplingRate * 2 + kTestSamplingRate / 100 + kTestWavePeriod - 1) / kTestWavePeriod * kTestWavePeriod;

	enum class Notes
	{
		No,
		Yes,
	};

	enum class Loop
	{
		No,
		Yes,
	};

	enum class Looping
	{
		No,
		Yes,
	};

	auto makeTestRenderer(Notes notes, Loop loop, Looping looping)
	{
		seir::synth::CompositionData composition;
		const auto voice = std::make_shared<seir::synth::VoiceData>();
		voice->_amplitudeEnvelope._changes.emplace_back(0ms, 1.f);
		voice->_amplitudeEnvelope._changes.emplace_back(1010ms, 1.f);
		voice->_asymmetryEnvelope._changes.emplace_back(0ms, 1.f);
		const auto& part = composition._parts.emplace_back(std::make_shared<seir::synth::PartData>(voice));
		const auto& track = part->_tracks.emplace_back(std::make_shared<seir::synth::TrackData>(std::make_shared<seir::synth::TrackProperties>()));
		const auto& sequence = track->_sequences.emplace_back(std::make_shared<seir::synth::SequenceData>());
		if (notes == Notes::Yes)
		{
			sequence->_sounds.emplace_back(0u, kTestNote, 0u);
			sequence->_sounds.emplace_back(1u, kTestNote, 0u);
		}
		track->_fragments.emplace(0u, sequence);
		if (loop == Loop::Yes)
		{
			composition._loopOffset = 1;
			composition._loopLength = 1;
		}
		return seir::synth::Renderer::create(*composition.pack(), { kTestSamplingRate, seir::synth::ChannelLayout::Mono }, looping == Looping::Yes);
	}

	size_t renderAction(seir::synth::Renderer& renderer, size_t frames)
	{
		static std::array<float, 32'768> buffer;
		return renderer.render(buffer.data(), frames);
	}

	size_t skipAction(seir::synth::Renderer& renderer, size_t frames)
	{
		return renderer.skipFrames(frames);
	}

	void expectEmpty(Notes notes, Loop loop, Looping looping, const std::function<size_t(seir::synth::Renderer&, size_t)>& action)
	{
		const auto renderer = ::makeTestRenderer(notes, loop, looping);
		CHECK(renderer->loopOffset() == 0);
		CHECK(renderer->currentOffset() == 0);
		CHECK(action(*renderer, 1) == 0);
		CHECK(renderer->currentOffset() == 0);
	}

	void expectLoop(Notes notes, Loop loop, Looping looping, const std::function<size_t(seir::synth::Renderer&, size_t)>& action, size_t loopStart, size_t loopEnd)
	{
		const auto renderer = ::makeTestRenderer(notes, loop, looping);
		CHECK(renderer->loopOffset() == loopStart);
		CHECK(renderer->currentOffset() == 0);
		CHECK(action(*renderer, loopEnd - 2) == loopEnd - 2);
		CHECK(renderer->currentOffset() == loopEnd - 2);
		CHECK(action(*renderer, 1) == 1);
		CHECK(renderer->currentOffset() == loopEnd - 1);
		CHECK(action(*renderer, 1) == 1);
		CHECK(renderer->currentOffset() == loopStart);
		CHECK(action(*renderer, 1) == 1);
		CHECK(renderer->currentOffset() == loopStart + 1);
		CHECK(action(*renderer, loopEnd - loopStart + 1) == loopEnd - loopStart + 1);
		CHECK(renderer->currentOffset() == loopStart + 2);
	}

	void expectNoLoop(Notes notes, Loop loop, Looping looping, const std::function<size_t(seir::synth::Renderer&, size_t)>& action)
	{
		const auto renderer = ::makeTestRenderer(notes, loop, looping);
		CHECK(renderer->loopOffset() == 0);
		CHECK(renderer->currentOffset() == 0);
		CHECK(action(*renderer, kTestSamples - 1) == kTestSamples - 1);
		CHECK(renderer->currentOffset() == kTestSamples - 1);
		CHECK(action(*renderer, 1) == 1);
		CHECK(renderer->currentOffset() == kTestSamples);
		CHECK(action(*renderer, 1) == 0);
		CHECK(renderer->currentOffset() == kTestSamples);
	}
}

TEST_CASE("Render (no notes, no loop, no looping)")
{
	expectEmpty(Notes::No, Loop::No, Looping::No, renderAction);
}

TEST_CASE("Render (with notes, no loop, no looping)")
{
	expectNoLoop(Notes::Yes, Loop::No, Looping::No, renderAction);
}

TEST_CASE("Render (no notes, with loop, no looping)")
{
	expectEmpty(Notes::No, Loop::Yes, Looping::No, renderAction);
}

TEST_CASE("Render (with notes, with loop, no looping)")
{
	expectNoLoop(Notes::Yes, Loop::Yes, Looping::No, renderAction);
}

TEST_CASE("Render (no notes, no loop, with looping)")
{
	expectLoop(Notes::No, Loop::No, Looping::Yes, renderAction, 0, kTestSamplingRate);
}

TEST_CASE("Render (with notes, no loop, with looping)")
{
	expectLoop(Notes::Yes, Loop::No, Looping::Yes, renderAction, 0, kTestSamplingRate * 3);
}

TEST_CASE("Render (no notes, with loop, with looping)")
{
	expectLoop(Notes::No, Loop::Yes, Looping::Yes, renderAction, kTestSamplingRate, kTestSamplingRate * 2);
}

TEST_CASE("Render (with notes, with loop, with looping)")
{
	expectLoop(Notes::Yes, Loop::Yes, Looping::Yes, renderAction, kTestSamplingRate, kTestSamplingRate * 2);
}

TEST_CASE("Skip (no notes, no loop, no looping)")
{
	expectEmpty(Notes::No, Loop::No, Looping::No, skipAction);
}

TEST_CASE("Skip (with notes, no loop, no looping)")
{
	expectNoLoop(Notes::Yes, Loop::No, Looping::No, skipAction);
}

TEST_CASE("Skip (no notes, with loop, no looping)")
{
	expectEmpty(Notes::No, Loop::Yes, Looping::No, skipAction);
}

TEST_CASE("Skip (with notes, with loop, no looping)")
{
	expectNoLoop(Notes::Yes, Loop::Yes, Looping::No, skipAction);
}

TEST_CASE("Skip (no notes, no loop, with looping)")
{
	expectLoop(Notes::No, Loop::No, Looping::Yes, skipAction, 0, kTestSamplingRate);
}

TEST_CASE("Skip (with notes, no loop, with looping)")
{
	expectLoop(Notes::Yes, Loop::No, Looping::Yes, skipAction, 0, kTestSamplingRate * 3);
}

TEST_CASE("Skip (no notes, with loop, with looping)")
{
	expectLoop(Notes::No, Loop::Yes, Looping::Yes, skipAction, kTestSamplingRate, kTestSamplingRate * 2);
}

TEST_CASE("Skip (with notes, with loop, with looping)")
{
	expectLoop(Notes::Yes, Loop::Yes, Looping::Yes, skipAction, kTestSamplingRate, kTestSamplingRate * 2);
}
