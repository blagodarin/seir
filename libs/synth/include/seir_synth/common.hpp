// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <chrono>
#include <vector>

namespace seir::synth
{
	static constexpr size_t kNotesPerOctave = 12;
	static constexpr size_t kOctaveCount = 9; // Subcontra octave (0-th) to five-lined octave (8-th).
	static constexpr size_t kNoteCount = kOctaveCount * kNotesPerOctave;

	enum class Note : uint8_t
	{
		// clang-format off
		C0, Db0, D0, Eb0, E0, F0, Gb0, G0, Ab0, A0, Bb0, B0,
		C1, Db1, D1, Eb1, E1, F1, Gb1, G1, Ab1, A1, Bb1, B1,
		C2, Db2, D2, Eb2, E2, F2, Gb2, G2, Ab2, A2, Bb2, B2,
		C3, Db3, D3, Eb3, E3, F3, Gb3, G3, Ab3, A3, Bb3, B3,
		C4, Db4, D4, Eb4, E4, F4, Gb4, G4, Ab4, A4, Bb4, B4,
		C5, Db5, D5, Eb5, E5, F5, Gb5, G5, Ab5, A5, Bb5, B5,
		C6, Db6, D6, Eb6, E6, F6, Gb6, G6, Ab6, A6, Bb6, B6,
		C7, Db7, D7, Eb7, E7, F7, Gb7, G7, Ab7, A7, Bb7, B7,
		C8, Db8, D8, Eb8, E8, F8, Gb8, G8, Ab8, A8, Bb8, B8,
		// clang-format on
	};

	static constexpr size_t kMaxSustain = 255;

	struct Sound
	{
		size_t _delay = 0; // Offset from the previous sound in a sequence.
		Note _note = Note::C0;
		size_t _sustain = 0;

		constexpr Sound(size_t delay, Note note, size_t sustain) noexcept
			: _delay{ delay }, _note{ note }, _sustain{ sustain } {}
	};

	// Shape types.
	enum class WaveShape
	{
		Linear,      // Straight line (used for synthesizing square, rectangular, sawtooth and triangle waves).
		Quadratic,   // Quadratic curve with parameterized derivative at the left end.
		Quadratic2,  // Two quadratic curves with parameterized derivatives at the ends and a common point in the middle.
		Cubic,       // Cubic curve with parameterized derivatives at the ends.
		Cubic2,      // Cubic curve with parameterized derivatives at the ends.
		Quintic,     // Quintic curve with zero value and parameterized derivative in the middle.
		Cosine,      // Cosine curve.
		CosineCubed, // Cosine cubed curve.
	};

	struct WaveShapeParameters
	{
		float _shape1 = 0;
		float _shape2 = 0;
	};

	struct EnvelopeChange
	{
		static constexpr auto kMaxDuration = std::chrono::milliseconds{ 60'000 };

		std::chrono::milliseconds _duration{ 0 };
		float _value = 0.f;

		constexpr EnvelopeChange(const std::chrono::milliseconds& duration, float value) noexcept
			: _duration{ duration }, _value{ value } {}
	};

	// Specifies how a value changes over time.
	struct Envelope
	{
		std::vector<EnvelopeChange> _changes; // List of consecutive value changes.
		size_t _sustainIndex = 0;
	};

	struct Oscillation
	{
		float _frequency = 1.f;
		float _magnitude = 0.f;
	};

	// Specifies how to generate a waveform for a sound.
	struct VoiceData
	{
		WaveShape _waveShape = WaveShape::Linear;
		WaveShapeParameters _waveShapeParameters;
		Envelope _amplitudeEnvelope;
		Oscillation _tremolo;
		Envelope _frequencyEnvelope;
		Oscillation _vibrato;
		Envelope _asymmetryEnvelope;
		Oscillation _asymmetryOscillation;
		Envelope _rectangularityEnvelope;
		Oscillation _rectangularityOscillation;
	};

	enum class Polyphony
	{
		Chord, // Multiple notes which start simultaneously are rendered as a chord.
		Full,  // All distinct notes are rendered independently.
	};

	struct TrackProperties
	{
		unsigned _weight = 1;
		Polyphony _polyphony = Polyphony::Chord;
		float _headDelay = 2.f;      // In milliseconds.
		float _sourceDistance = 2.f; // In head radiuses.
		unsigned _sourceWidth = 180; // In degrees.
		int _sourceOffset = 0;       // In degrees, zero is forward, positive is right.
	};

	constexpr unsigned kMinSpeed = 1;  // Minimum composition playback speed (in steps per second).
	constexpr unsigned kMaxSpeed = 32; // Maximum composition playback speed (in steps per second).
}
