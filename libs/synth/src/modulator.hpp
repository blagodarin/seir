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
		explicit constexpr Modulator(std::span<const SampledPoint> points) noexcept
			: _points{ points.data() }
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
				const auto remainingDelay = nextPoint._delaySamples - _offsetSamples;
				if (remainingDelay > samples)
				{
					_offsetSamples += samples;
					_currentValue = _lastPointValue + (nextPoint._value - _lastPointValue) * _offsetSamples / nextPoint._delaySamples;
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

		constexpr auto currentValue() const noexcept
		{
			return _currentValue;
		}

		constexpr auto maxContinuousAdvance() const noexcept
		{
			return _points[_nextIndex]._delaySamples - _offsetSamples;
		}

		void start(float offsetSamples) noexcept
		{
			assert(offsetSamples >= 0);
			_nextIndex = 1;
			_lastPointValue = _points->_value;
			for (;;)
			{
				if (_nextIndex == _size)
				{
					_offsetSamples = 0;
					_currentValue = _lastPointValue;
					break;
				}
				const auto& nextPoint = _points[_nextIndex];
				if (nextPoint._delaySamples > offsetSamples)
				{
					_offsetSamples = offsetSamples;
					_currentValue = _lastPointValue + (nextPoint._value - _lastPointValue) * _offsetSamples / nextPoint._delaySamples;
					break;
				}
				offsetSamples -= nextPoint._delaySamples;
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

		constexpr bool stopped() const noexcept
		{
			return _nextIndex == _size;
		}

	private:
		const SampledPoint* const _points;
		const unsigned _size;
		unsigned _nextIndex = _size;
		float _lastPointValue = _points[_size]._value;
		float _offsetSamples = 0;
		float _currentValue = _lastPointValue;
	};
}
