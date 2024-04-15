// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_base/clock.hpp>

#include <doctest/doctest.h>

namespace
{
	using namespace std::literals::chrono_literals;

	class ClockMock
	{
	public:
		using duration = std::chrono::microseconds;
		using period = std::micro;
		using rep = duration::rep;
		using time_point = std::chrono::time_point<ClockMock, duration>;

		static constexpr bool is_steady = true;

		static time_point now() { return _now; }

		constexpr ClockMock(const duration& d) noexcept
		{
			_now = time_point{ d };
		}

		constexpr void advance(const duration& d) noexcept
		{
			_now += d;
		}

	private:
		static time_point _now;
	};

	ClockMock::time_point ClockMock::_now;

	static_assert(std::chrono::is_clock_v<ClockMock>);

	class FrameClock
	{
	public:
		FrameClock()
		{
			REQUIRE(_clock.seconds() == 0.f);
		}

		template <typename T>
		std::optional<seir::FrameClock<ClockMock>::Period> tick(const ClockMock::duration& duration, T&& seconds)
		{
			_mock.advance(duration);
			const auto period = _clock.tick();
			REQUIRE(_clock.seconds() == seconds);
			return period;
		}

	private:
		ClockMock _mock{ 999'999'999us };
		seir::FrameClock<ClockMock> _clock;
	};
}

TEST_CASE("FrameClock")
{
	using doctest::Approx;
	FrameClock clock;
	SUBCASE("0s + 900'900us + 99'099us + 1us + 0s + 999'999us + 1us")
	{
		REQUIRE_FALSE(clock.tick(0s, 0.f));
		REQUIRE_FALSE(clock.tick(900'900us, Approx{ 0.900'900f }));
		REQUIRE_FALSE(clock.tick(99'099us, Approx{ 0.999'999f }));
		auto period = clock.tick(1us, 1.f);
		REQUIRE(period);
		CHECK(period->_frameCount == 4);
		CHECK(period->_framesPerSecond == 4.f);
		CHECK(period->_maxFrameMilliseconds == 901);

		REQUIRE_FALSE(clock.tick(0s, 1.f));
		REQUIRE_FALSE(clock.tick(999'999us, Approx{ 1.999'999f }));
		period = clock.tick(1us, 2.f);
		REQUIRE(period);
		CHECK(period->_frameCount == 3);
		CHECK(period->_framesPerSecond == 3.f);
		CHECK(period->_maxFrameMilliseconds == 1000);
	}
	SUBCASE("600ms + 900ms + 999'999us + 1us")
	{
		REQUIRE_FALSE(clock.tick(600ms, Approx{ 0.6f }));
		auto period = clock.tick(900ms, 1.5f);
		REQUIRE(period);
		CHECK(period->_frameCount == 2);
		CHECK(period->_framesPerSecond == Approx{ 2.f / 1.5f });
		CHECK(period->_maxFrameMilliseconds == 900);

		REQUIRE_FALSE(clock.tick(999'999us, Approx{ 2.499'999f }));
		period = clock.tick(1us, 2.5f);
		REQUIRE(period);
		CHECK(period->_frameCount == 2);
		CHECK(period->_framesPerSecond == 2.f);
		CHECK(period->_maxFrameMilliseconds == 1000);
	}
}
