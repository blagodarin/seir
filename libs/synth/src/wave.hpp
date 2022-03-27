// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/rigid_vector.hpp>
#include <seir_synth/common.hpp>
#include "modulator.hpp"
#include "oscillator.hpp"
#include "period.hpp"

namespace seir::synth
{
	enum class Transformation
	{
		None,
		Exp2,
	};

	template <Transformation>
	float transform(float) noexcept = delete;

	template <>
	constexpr float transform<Transformation::None>(float value) noexcept { return value; }

	template <>
	inline float transform<Transformation::Exp2>(float value) noexcept { return std::exp2(value); }

	class WaveData
	{
	public:
		WaveData(const VoiceData& data, unsigned samplingRate)
			: _shapeParameter{ data._waveShapeParameter }
			, _amplitudeSize{ 1 + static_cast<unsigned>(data._amplitudeEnvelope._changes.size()) + 1 }
			, _frequencyOffset{ _amplitudeSize + 1 }
			, _frequencySize{ 1 + static_cast<unsigned>(data._frequencyEnvelope._changes.size()) + 1 }
			, _asymmetryOffset{ _frequencyOffset + _frequencySize + 1 }
			, _asymmetrySize{ 1 + static_cast<unsigned>(data._asymmetryEnvelope._changes.size()) + 1 }
			, _rectangularityOffset{ _asymmetryOffset + _asymmetrySize + 1 }
			, _rectangularitySize{ 1 + static_cast<unsigned>(data._rectangularityEnvelope._changes.size()) + 1 }
			, _tremolo{ data._tremolo }
			, _vibrato{ data._vibrato }
			, _asymmetryOscillation{ data._asymmetryOscillation }
			, _rectangularityOscillation{ data._rectangularityOscillation }
		{
			_pointBuffer.reserve(size_t{ _rectangularityOffset } + _rectangularitySize + 1);
			addPoints<Transformation::None>(data._amplitudeEnvelope, _amplitudeSustainIndex, samplingRate);
			addPoints<Transformation::Exp2>(data._frequencyEnvelope, _frequencySustainIndex, samplingRate);
			addPoints<Transformation::None>(data._asymmetryEnvelope, _asymmetrySustainIndex, samplingRate);
			addPoints<Transformation::None>(data._rectangularityEnvelope, _rectangularitySustainIndex, samplingRate);
		}

		[[nodiscard]] std::span<const SampledPoint> amplitudePoints() const noexcept { return { _pointBuffer.data(), _amplitudeSize }; }
		[[nodiscard]] constexpr auto amplitudeSustainIndex() const noexcept { return _amplitudeSustainIndex; }
		[[nodiscard]] constexpr auto& asymmetryOscillation() const noexcept { return _asymmetryOscillation; }
		[[nodiscard]] std::span<const SampledPoint> asymmetryPoints() const noexcept { return { _pointBuffer.data() + _asymmetryOffset, _asymmetrySize }; }
		[[nodiscard]] constexpr auto asymmetrySustainIndex() const noexcept { return _asymmetrySustainIndex; }
		[[nodiscard]] std::span<const SampledPoint> frequencyPoints() const noexcept { return { _pointBuffer.data() + _frequencyOffset, _frequencySize }; }
		[[nodiscard]] constexpr auto frequencySustainIndex() const noexcept { return _frequencySustainIndex; }
		[[nodiscard]] constexpr auto& rectangularityOscillation() const noexcept { return _rectangularityOscillation; }
		[[nodiscard]] std::span<const SampledPoint> rectangularityPoints() const noexcept { return { _pointBuffer.data() + _rectangularityOffset, _rectangularitySize }; }
		[[nodiscard]] constexpr auto rectangularitySustainIndex() const noexcept { return _rectangularitySustainIndex; }
		[[nodiscard]] constexpr auto shapeParameter() const noexcept { return _shapeParameter; }
		[[nodiscard]] constexpr auto& tremolo() const noexcept { return _tremolo; }
		[[nodiscard]] constexpr auto& vibrato() const noexcept { return _vibrato; }

	private:
		template <Transformation transformation>
		void addPoints(const seir::synth::Envelope& envelope, size_t& sustainIndex, unsigned samplingRate)
		{
			const auto insertSustainIndex = envelope._sustainIndex > 0 ? envelope._sustainIndex - 1 : envelope._changes.size();
			auto value = transform<transformation>(0);
			_pointBuffer.emplace_back(0u, value);
			for (size_t i = 0; i < envelope._changes.size(); ++i)
			{
				value = transform<transformation>(envelope._changes[i]._value);
				_pointBuffer.emplace_back(static_cast<unsigned>(envelope._changes[i]._duration.count() * samplingRate / 1000), value);
				if (i == insertSustainIndex)
					_pointBuffer.emplace_back(0u, value);
			}
			if (insertSustainIndex >= envelope._changes.size())
				_pointBuffer.emplace_back(0u, value);
			sustainIndex = insertSustainIndex + 1;
			_pointBuffer.emplace_back(std::numeric_limits<unsigned>::max(), value);
		}

	private:
		const float _shapeParameter;
		const unsigned _amplitudeSize;
		const unsigned _frequencyOffset;
		const unsigned _frequencySize;
		const unsigned _asymmetryOffset;
		const unsigned _asymmetrySize;
		const unsigned _rectangularityOffset;
		const unsigned _rectangularitySize;
		seir::RigidVector<SampledPoint> _pointBuffer;
		size_t _amplitudeSustainIndex = 0;
		size_t _frequencySustainIndex = 0;
		size_t _asymmetrySustainIndex = 0;
		size_t _rectangularitySustainIndex = 0;
		Oscillation _tremolo;
		Oscillation _vibrato;
		Oscillation _asymmetryOscillation;
		Oscillation _rectangularityOscillation;
	};

	class WaveState
	{
	public:
		WaveState(const WaveData& data, unsigned samplingRate) noexcept
			: _samplingRate{ static_cast<float>(samplingRate) }
			, _shapeParameter{ data.shapeParameter() }
			, _amplitudeModulator{ data.amplitudePoints(), data.amplitudeSustainIndex() }
			, _amplitudeOscillator{ data.tremolo()._frequency / _samplingRate, data.tremolo()._magnitude }
			, _frequencyModulator{ data.frequencyPoints(), data.frequencySustainIndex() }
			, _frequencyOscillator{ data.vibrato()._frequency / _samplingRate, 1 - std::exp2(-data.vibrato()._magnitude) }
			, _asymmetryModulator{ data.asymmetryPoints(), data.asymmetrySustainIndex() }
			, _asymmetryOscillator{ data.asymmetryOscillation()._frequency / _samplingRate, data.asymmetryOscillation()._magnitude }
			, _rectangularityModulator{ data.rectangularityPoints(), data.rectangularitySustainIndex() }
			, _rectangularityOscillator{ data.rectangularityOscillation()._frequency / _samplingRate, data.rectangularityOscillation()._magnitude }
		{
		}

		void advance(int samples) noexcept
		{
			assert(samples > 0);
			if (!_period.stopped())
				_period.advance(static_cast<float>(samples));
			if (_needRestart)
				_restartDelay -= samples;
		}

		[[nodiscard]] int prepareAdvance() noexcept
		{
			if (_period.stopped())
			{
				if (_needRestart && _restartDelay <= 0)
				{
					_needRestart = false;
					startWave(_restartFrequency, _restartAmplitude, _restartSustain, static_cast<float>(-_restartDelay));
				}
				else
				{
					if (_amplitudeModulator.stopped())
					{
						_period = {};
						return _needRestart ? _restartDelay : std::numeric_limits<int>::max();
					}
					_offset += _periodLength;
					startWavePeriod();
				}
			}
			return static_cast<int>(std::ceil(_period.maxAdvance()));
		}

		[[nodiscard]] constexpr ShaperData shaperData() const noexcept
		{
			return _period.shaperData(_periodRectangularity, _shapeParameter);
		}

		void start(float frequency, float amplitude, float sustain, int delay) noexcept
		{
			assert(frequency > 0);
			assert(amplitude > 0);
			assert(delay >= 0);
			assert(!_needRestart); // TODO: Come up with a way to handle frequent wave restarts.
			if (_period.stopped() && delay == 0)
			{
				startWave(frequency, amplitude, sustain, 0);
			}
			else
			{
				_needRestart = true;
				_restartDelay = delay;
				_restartFrequency = frequency;
				_restartAmplitude = amplitude;
				_restartSustain = sustain;
			}
		}

		constexpr void stop() noexcept
		{
			_amplitudeModulator.stop();
			_period = {};
			_needRestart = false;
		}

	private:
		[[nodiscard]] static constexpr float adjust(float value, float adjustment) noexcept
		{
			assert(value >= 0.f && value <= 1.f);
			assert(adjustment >= 0.f && adjustment <= 1.f);
			return value + (1 - value) * adjustment;
		}

		void startWave(float frequency, float amplitude, float sustainSamples, float offsetSamples) noexcept
		{
			assert(frequency > 0);
			assert(amplitude > 0);
			assert(offsetSamples >= 0);
			_amplitudeModulator.start(sustainSamples, offsetSamples);
			_frequencyModulator.start(sustainSamples, offsetSamples);
			_asymmetryModulator.start(sustainSamples, offsetSamples);
			_rectangularityModulator.start(sustainSamples, offsetSamples);
			_frequency = frequency;
			_amplitude = amplitude;
			_offset = offsetSamples;
			_periodLength = 0;
			startWavePeriod();
		}

		void startWavePeriod() noexcept
		{
			const auto periodFrequency = _frequency * _frequencyModulator.advance(_periodLength) * (1 - _frequencyOscillator.value(_offset));
			assert(periodFrequency > 0);
			_periodLength = _samplingRate / periodFrequency;
			const auto periodAmplitude = _amplitude * _amplitudeModulator.advance(_periodLength) * (1 - _amplitudeOscillator.value(_offset));
			const auto periodAsymmetry = adjust(_asymmetryModulator.advance(_periodLength), _asymmetryOscillator.value(_offset));
			_periodRectangularity = adjust(_rectangularityModulator.advance(_periodLength), _rectangularityOscillator.value(_offset));
			_period.start(_periodLength, periodAmplitude, periodAsymmetry, _amplitudeModulator.stopped());
		}

	private:
		const float _samplingRate;
		const float _shapeParameter;
		Modulator _amplitudeModulator;
		TriangleOscillator _amplitudeOscillator;
		Modulator _frequencyModulator;
		TriangleOscillator _frequencyOscillator;
		Modulator _asymmetryModulator;
		TriangleOscillator _asymmetryOscillator;
		Modulator _rectangularityModulator;
		TriangleOscillator _rectangularityOscillator;
		WavePeriod _period;
		float _offset = 0;
		float _periodLength = 0;
		float _periodRectangularity = 0;
		float _frequency = 0;
		float _amplitude = 0;
		bool _needRestart = false;
		int _restartDelay = 0;
		float _restartFrequency = 0;
		float _restartAmplitude = 0;
		float _restartSustain = 0;
	};
}
