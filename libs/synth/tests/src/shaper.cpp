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
	void checkShaper(int precisionBits, float shapeParameter)
	{
		INFO("Shape = " << shapeParameter << ", Precision = " << precisionBits);
		constexpr auto amplitude = 1.f;
		constexpr auto range = 2 * amplitude;
		const auto precision = std::ldexp(range, -precisionBits);
		constexpr auto minFrequency = seir::synth::kNoteFrequencies[seir::synth::Note::C0] / 2; // Lowest note at lowest frequency modulation.
		constexpr auto deltaX = seir::synth::Renderer::kMaxSamplingRate / minFrequency;         // Asymmetric wave of minimum frequency at highest supported sampling rate.
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
	::checkShaper<seir::synth::CosineShaper>(23, {});
}

TEST_CASE("shaper_cubic")
{
	::checkShaper<seir::synth::CubicShaper>(23, 0.f);
	::checkShaper<seir::synth::CubicShaper>(22, 3.f);
	::checkShaper<seir::synth::CubicShaper>(21, 3.5f);
	::checkShaper<seir::synth::CubicShaper>(20, 6.f);
	::checkShaper<seir::synth::CubicShaper>(19, 7.f);
	::checkShaper<seir::synth::CubicShaper>(18, 7.75f);
	::checkShaper<seir::synth::CubicShaper>(19, 8.f);
	::checkShaper<seir::synth::CubicShaper>(20, 8.98f);
}

TEST_CASE("shaper_linear")
{
	::checkShaper<seir::synth::LinearShaper>(23, {});
}

TEST_CASE("shaper_quadratic")
{
	::checkShaper<seir::synth::QuadraticShaper>(23, 0.f);
	::checkShaper<seir::synth::QuadraticShaper>(23, 1.f);
	::checkShaper<seir::synth::QuadraticShaper>(23, 2.f);
}

TEST_CASE("shaper_quadratic2")
{
	::checkShaper<seir::synth::Quadratic2Shaper>(23, 0.f);
	::checkShaper<seir::synth::Quadratic2Shaper>(23, 3.f);
	::checkShaper<seir::synth::Quadratic2Shaper>(22, 6.f);
	::checkShaper<seir::synth::Quadratic2Shaper>(21, 6.82f);
}

TEST_CASE("shaper_quintic")
{
	::checkShaper<seir::synth::QuinticShaper>(23, -1.5f);
	::checkShaper<seir::synth::QuinticShaper>(22, -1.375f);
	::checkShaper<seir::synth::QuinticShaper>(21, -1.25f);
	::checkShaper<seir::synth::QuinticShaper>(20, -1.f);
	::checkShaper<seir::synth::QuinticShaper>(19, 0.f);
	::checkShaper<seir::synth::QuinticShaper>(18, 1.f);
	//::checkShaper<seir::synth::QuinticShaper>(17, 1.5f); // TODO: Fix out-of-range output.
	::checkShaper<seir::synth::QuinticShaper>(17, 3.f);
	::checkShaper<seir::synth::QuinticShaper>(16, 4.01f);
}
