// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cassert>
#include <chrono>
#include <optional>

namespace seir
{
	// Variable frame rate clock.
	template <typename Clock = std::chrono::steady_clock>
	requires std::chrono::is_clock_v<Clock>
	class FrameClock
	{
	public:
		struct Period
		{
			unsigned _frameCount = 0;       // Number of frames in the period.
			float _averageFps = 0;          // Average frame rate during the period.
			unsigned _peakMilliseconds = 0; // Peak frame duration in milliseconds, rounded up.
		};

		[[nodiscard]] float seconds() const noexcept
		{
			return std::chrono::duration_cast<std::chrono::duration<float, std::chrono::seconds::period>>(_lastTickTime - _startTime).count();
		}

		[[nodiscard]] std::optional<Period> tick() noexcept
		{
			const auto now = Clock::now();
			const auto frameDuration = now - _lastTickTime;
			_lastTickTime = now;
			if (frameDuration > _maxFrameDuration)
				_maxFrameDuration = frameDuration;
			constexpr auto periodDurationLimit = std::chrono::seconds{ 1 };
			assert(_periodDuration < periodDurationLimit);
			_periodDuration += frameDuration;
			++_framesInPeriod;
			if (_periodDuration < periodDurationLimit)
				return {};
			const auto periodsInSecond = Clock::period::den / (std::chrono::duration_cast<std::chrono::duration<float, typename Clock::period>>(_periodDuration).count() * Clock::period::num);
			assert(periodsInSecond <= 1.f);
			Period period{
				._frameCount = _framesInPeriod,
				._averageFps = static_cast<float>(_framesInPeriod) * periodsInSecond,
				._peakMilliseconds = static_cast<unsigned>(std::chrono::ceil<std::chrono::milliseconds>(_maxFrameDuration).count()),
			};
			_maxFrameDuration = Clock::duration::zero();
			_periodDuration = Clock::duration::zero();
			_framesInPeriod = 0;
			return period;
		}

	private:
		const typename Clock::time_point _startTime = Clock::now();
		Clock::time_point _lastTickTime = _startTime;
		Clock::duration _maxFrameDuration = Clock::duration::zero();
		Clock::duration _periodDuration = Clock::duration::zero();
		unsigned _framesInPeriod = 0;
	};
}
