// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cassert>
#include <chrono>
#include <optional>

// TODO: Remove '#ifndef __APPLE__' when P0355 is implemented: https://libcxx.llvm.org/Status/Cxx20.html

namespace seir
{
	// Constant frame rate clock.
	template <typename Clock = std::chrono::steady_clock>
#ifndef __APPLE__
	requires std::chrono::is_clock_v<Clock>
#endif
	class ConstantRate
	{
	public:
		// Creates the clock with the specified interval.
		// The clock is not started after construction.
		constexpr ConstantRate(const typename Clock::duration& interval) noexcept
			: _interval{ interval } { assert(_interval.count() > 0); }

		// Returns the number of new frames since the last call
		// if the clock is started, starts the clock and returns zero if not.
		[[nodiscard]] unsigned advance() noexcept;

		// Resets the clock to the initial (non-started) state.
		constexpr void reset() noexcept;

		// Starts (or restarts) the clock.
		void start() noexcept;

	private:
		const typename Clock::duration _interval;
		typename Clock::time_point _base;
		bool _started = false;
	};

	// Variable period metrics.
	struct VariablePeriod
	{
		unsigned _frameCount = 0;       // Number of frames in the period.
		float _averageFrameRate = 0;    // Average frame rate during the period.
		unsigned _maxFrameDuration = 0; // Maximum frame duration in milliseconds, rounded up.
	};

	// Variable frame rate clock, useful for FPS measurement.
	template <typename Clock = std::chrono::steady_clock>
#ifndef __APPLE__
	requires std::chrono::is_clock_v<Clock>
#endif
	class VariableRate
	{
	public:
		// Advances the clock for the next frame.
		// Returns period metrics if enough data is collected.
		[[nodiscard]] std::optional<VariablePeriod> advance() noexcept;

		// Resets the clock to the initial (non-started) state.
		constexpr void reset() noexcept;

		// Starts (or restarts) the clock.
		void start() noexcept;

		// Returns the accounted time in seconds.
		[[nodiscard]] constexpr float time() const noexcept;

	private:
		typename Clock::time_point _startTime;
		typename Clock::time_point _lastFrameTime;
		typename Clock::duration _maxFrameDuration = Clock::duration::zero();
		typename Clock::duration _periodDuration = Clock::duration::zero();
		unsigned _framesInPeriod = 0;
	};
}

template <typename Clock>
#ifndef __APPLE__
requires std::chrono::is_clock_v<Clock>
#endif
unsigned seir::ConstantRate<Clock>::advance() noexcept
{
	const auto now = Clock::now();
	if (!_started) [[unlikely]]
	{
		_base = now;
		_started = true;
		return 0;
	}
	const auto frames = (now - _base).count() / _interval.count();
	_base += frames * _interval;
	return static_cast<unsigned>(frames);
}

template <typename Clock>
#ifndef __APPLE__
requires std::chrono::is_clock_v<Clock>
#endif
constexpr void seir::ConstantRate<Clock>::reset() noexcept
{
	_started = false;
}

template <typename Clock>
#ifndef __APPLE__
requires std::chrono::is_clock_v<Clock>
#endif
void seir::ConstantRate<Clock>::start() noexcept
{
	_base = Clock::now();
	_started = true;
}

template <typename Clock>
#ifndef __APPLE__
requires std::chrono::is_clock_v<Clock>
#endif
std::optional<seir::VariablePeriod> seir::VariableRate<Clock>::advance() noexcept
{
	const auto now = Clock::now();
	if (_startTime == decltype(_startTime){}) [[unlikely]]
	{
		assert(now != decltype(_startTime){});
		_startTime = now;
		_lastFrameTime = now;
		return {};
	}
	const auto frameDuration = now - _lastFrameTime;
	_lastFrameTime = now;
	if (frameDuration > _maxFrameDuration)
		_maxFrameDuration = frameDuration;
	constexpr auto periodDurationLimit = std::chrono::seconds{ 1 };
	assert(_periodDuration < periodDurationLimit);
	_periodDuration += frameDuration;
	++_framesInPeriod;
	if (_periodDuration < periodDurationLimit) [[likely]]
		return {};
	const auto periodsInSecond = Clock::period::den / (std::chrono::duration_cast<std::chrono::duration<float, typename Clock::period>>(_periodDuration).count() * Clock::period::num);
	assert(periodsInSecond <= 1.f);
	VariablePeriod period{
		._frameCount = _framesInPeriod,
		._averageFrameRate = static_cast<float>(_framesInPeriod) * periodsInSecond,
		._maxFrameDuration = static_cast<unsigned>(std::chrono::ceil<std::chrono::milliseconds>(_maxFrameDuration).count()),
	};
	_maxFrameDuration = Clock::duration::zero();
	_periodDuration = Clock::duration::zero();
	_framesInPeriod = 0;
	return period;
}

template <typename Clock>
#ifndef __APPLE__
requires std::chrono::is_clock_v<Clock>
#endif
constexpr void seir::VariableRate<Clock>::reset() noexcept
{
	_startTime = {};
	_lastFrameTime = {};
}

template <typename Clock>
#ifndef __APPLE__
requires std::chrono::is_clock_v<Clock>
#endif
void seir::VariableRate<Clock>::start() noexcept
{
	_startTime = Clock::now();
	_lastFrameTime = _startTime;
}

template <typename Clock>
#ifndef __APPLE__
requires std::chrono::is_clock_v<Clock>
#endif
constexpr float seir::VariableRate<Clock>::time() const noexcept
{
	return std::chrono::duration_cast<std::chrono::duration<float, std::chrono::seconds::period>>(_lastFrameTime - _startTime).count();
}
