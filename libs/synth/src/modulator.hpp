// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cassert>
#include <span>

namespace seir::synth
{
	struct SampledPoint
	{
		float _delaySamples;
		float _value;

		constexpr SampledPoint(unsigned delaySamples, float value) noexcept
			: _delaySamples{ static_cast<float>(delaySamples) }, _value{ value } {}
	};

	class Modulator
	{
	public:
		explicit constexpr Modulator(std::span<const SampledPoint> points, size_t sustainIndex) noexcept
			: _points{ points.data() }
			, _sustainNextIndex{ sustainIndex + 1 }
			, _size{ static_cast<unsigned>(points.size()) }
		{
			assert(_size >= 1);
			assert(_points->_delaySamples == 0.f);
		}

		[[nodiscard]] constexpr float advance(float samples) noexcept
		{
			auto maxValue = _currentValue;
			for (; _nextIndex < _size; ++_nextIndex)
			{
				const auto& nextPoint = _points[_nextIndex];
				const auto nextPointDelay = _nextIndex == _sustainNextIndex ? _sustainSamples : nextPoint._delaySamples;
				const auto remainingDelay = nextPointDelay - _offsetSamples;
				if (remainingDelay > samples)
				{
					_offsetSamples += samples;
					_currentValue = _lastPointValue + (nextPoint._value - _lastPointValue) * _offsetSamples / nextPointDelay;
					break;
				}
				samples -= remainingDelay;
				_lastPointValue = nextPoint._value;
				_offsetSamples = 0;
				_currentValue = _lastPointValue;
			}
			if (_currentValue > maxValue)
				maxValue = _currentValue;
			return maxValue;
		}

		[[nodiscard]] constexpr auto currentValue() const noexcept
		{
			return _currentValue;
		}

		[[nodiscard]] constexpr auto maxContinuousAdvance() const noexcept
		{
			return _points[_nextIndex]._delaySamples - _offsetSamples;
		}

		void start(float sustainSamples, float offsetSamples) noexcept
		{
			assert(sustainSamples >= 0);
			assert(offsetSamples >= 0);
			_nextIndex = 1;
			_lastPointValue = _points->_value;
			_sustainSamples = sustainSamples;
			for (;;)
			{
				if (_nextIndex == _size)
				{
					_offsetSamples = 0;
					_currentValue = _lastPointValue;
					break;
				}
				const auto& nextPoint = _points[_nextIndex];
				const auto nextPointDelay = _nextIndex == _sustainNextIndex ? _sustainSamples : nextPoint._delaySamples;
				if (nextPointDelay > offsetSamples)
				{
					_offsetSamples = offsetSamples;
					_currentValue = _lastPointValue + (nextPoint._value - _lastPointValue) * _offsetSamples / nextPointDelay;
					break;
				}
				offsetSamples -= nextPointDelay;
				_lastPointValue = nextPoint._value;
				++_nextIndex;
			}
		}

		constexpr void stop() noexcept
		{
			_nextIndex = _size;
			_lastPointValue = _points[_size]._value;
			_offsetSamples = 0;
			_currentValue = _lastPointValue;
		}

		[[nodiscard]] constexpr bool stopped() const noexcept
		{
			return _nextIndex == _size;
		}

	private:
		const SampledPoint* const _points;
		const size_t _sustainNextIndex;
		const unsigned _size;
		unsigned _nextIndex = _size;
		float _lastPointValue = _points[_size]._value;
		float _sustainSamples = 0;
		float _offsetSamples = 0;
		float _currentValue = _lastPointValue;
	};
}
