// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_base/clock.hpp>

#include <doctest/doctest.h>

namespace
{
	class ClockMock
	{
	public:
		using duration = std::chrono::microseconds;
		using period = std::micro;
		using rep = duration::rep;
		using time_point = std::chrono::time_point<ClockMock, duration>;

		[[maybe_unused]] static constexpr bool is_steady = true;

		static time_point now() noexcept { return _now; }

		constexpr ClockMock(const duration& d) noexcept { _now = time_point{ d }; }
		constexpr void advance(const duration& d) noexcept { _now += d; }

	private:
		static time_point _now;
	};

	ClockMock::time_point ClockMock::_now;
}

TEST_CASE("FrameClock")
{
	using namespace std::literals::chrono_literals;
	using doctest::Approx;
	ClockMock mock{ 999'999'999us };
	seir::FrameClock<ClockMock> clock;
	REQUIRE(clock.seconds() == 0.f);
	const auto tick = [&mock, &clock](const ClockMock::duration& duration, auto&& expectedSeconds) {
		mock.advance(duration);
		auto period = clock.tick();
		REQUIRE(clock.seconds() == expectedSeconds);
		return period;
	};
	SUBCASE("0s + 999'001us + 998us + 1us + 999ms + 1ms")
	{
		// Frame durations aren't rounded neither up nor down,
		// but peak frame duration metric is rounded up.
		REQUIRE_FALSE(tick(0s, 0.f));
		REQUIRE_FALSE(tick(999'001us, Approx{ 0.999'001f }));
		REQUIRE_FALSE(tick(998us, Approx{ 0.999'999f }));
		auto period = tick(1us, 1.f);
		REQUIRE(period);
		CHECK(period->_frameCount == 4);
		CHECK(period->_averageFps == 4.f);
		CHECK(period->_peakMilliseconds == 1000);

		// Peak frame duration metric doesn't have an extra millisecond.
		REQUIRE_FALSE(tick(999ms, Approx{ 1.999f }));
		period = tick(1ms, 2.f);
		REQUIRE(period);
		CHECK(period->_frameCount == 2);
		CHECK(period->_averageFps == 2.f);
		CHECK(period->_peakMilliseconds == 999);
	}
	SUBCASE("250ms + 500ms + 750ms + 999'999us + 1us")
	{
		// The first period is longer than one second.
		REQUIRE_FALSE(tick(250ms, .25f));
		REQUIRE_FALSE(tick(500ms, .75f));
		auto period = tick(750ms, 1.5f);
		REQUIRE(period);
		CHECK(period->_frameCount == 3);
		CHECK(period->_averageFps == 2.f);
		CHECK(period->_peakMilliseconds == 750);

		// The second period is not affected by the preceding long period.
		REQUIRE_FALSE(tick(999'999us, Approx{ 2.499'999f }));
		period = tick(1us, 2.5f);
		REQUIRE(period);
		CHECK(period->_frameCount == 2);
		CHECK(period->_averageFps == 2.f);
		CHECK(period->_peakMilliseconds == 1000);
	}
}
