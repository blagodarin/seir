// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_synth/shaper.hpp>

#include <seir_synth/renderer.hpp>
#include "../../src/tables.hpp"

#include <doctest/doctest.h>

namespace
{
	template <typename Shaper>
	void checkShaper(float shapeParameter, int precisionBits)
	{
		INFO("Shape = " << shapeParameter << ", Precision = " << precisionBits);
		constexpr auto amplitude = 1.f;
		constexpr auto range = 2 * amplitude;
		const auto precision = std::ldexp(range, -precisionBits);
		const auto minFrequency = seir::synth::kNoteFrequencies[seir::synth::Note::C0] / 2; // Lowest note at lowest frequency modulation.
		const auto deltaX = seir::synth::Renderer::kMaxSamplingRate / minFrequency;         // Asymmetric wave of minimum frequency at highest supported sampling rate.
		Shaper shaper{ { amplitude, -range, deltaX, shapeParameter } };
		for (float i = 0; i < deltaX; ++i)
		{
			INFO("X = " << i << " / " << deltaX);
			const auto expected = Shaper::template value<double>(amplitude, -range, deltaX, shapeParameter, i);
			const auto initialValue = Shaper{ { amplitude, -range, deltaX, shapeParameter, i } }.advance();
			CHECK(std::abs(initialValue) <= amplitude);
			CHECK(initialValue == doctest::Approx{ expected }.epsilon(precision));
			const auto advancedValue = shaper.advance();
			CHECK(std::abs(advancedValue) <= amplitude);
			CHECK(advancedValue == doctest::Approx{ expected }.epsilon(precision));
		}
	}
}

TEST_CASE("shaper_cosine")
{
	::checkShaper<seir::synth::CosineShaper>({}, 23);
}

TEST_CASE("shaper_cubic")
{
	::checkShaper<seir::synth::CubicShaper>(0.f, 23);
	::checkShaper<seir::synth::CubicShaper>(3.f, 22);
	::checkShaper<seir::synth::CubicShaper>(8.98f, 20);
}

TEST_CASE("shaper_linear")
{
	::checkShaper<seir::synth::LinearShaper>({}, 23);
}

TEST_CASE("shaper_quadratic")
{
	::checkShaper<seir::synth::QuadraticShaper>(0.f, 23);
	::checkShaper<seir::synth::QuadraticShaper>(3.f, 23);
	::checkShaper<seir::synth::QuadraticShaper>(5.f, 21);
	::checkShaper<seir::synth::QuadraticShaper>(6.82f, 20);
}

TEST_CASE("shaper_quintic")
{
	::checkShaper<seir::synth::QuinticShaper>(-1.5f, 23);
	::checkShaper<seir::synth::QuinticShaper>(-1.f, 20);
	::checkShaper<seir::synth::QuinticShaper>(0.f, 19);
	::checkShaper<seir::synth::QuinticShaper>(1.f, 18);
	::checkShaper<seir::synth::QuinticShaper>(3.f, 17);
	::checkShaper<seir::synth::QuinticShaper>(4.01f, 16);
}
