// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_base/clock.hpp>

#include <doctest/doctest.h>

namespace
{
	class Clock
	{
	public:
		using duration = std::chrono::microseconds;
		using period = std::micro;
		using rep = duration::rep;
		using time_point = std::chrono::time_point<Clock, duration>;

		[[maybe_unused]] static constexpr bool is_steady = true; // cppcheck-suppress[unusedStructMember]

		static time_point now() noexcept { return _now; }

		constexpr explicit Clock(const duration& d) noexcept { _now = time_point{ d }; }
		constexpr void advance(const duration& d) noexcept { _now += d; }

	private:
		static time_point _now;
	};

	Clock::time_point Clock::_now;
}

TEST_CASE("ConstantRate")
{
	using namespace std::literals::chrono_literals;
	Clock clock{ 999'999'999us };
	seir::ConstantRate<Clock> rate{ 3ms };
	const auto advance = [&clock, &rate](const Clock::duration& duration, unsigned expectedFrames) {
		clock.advance(duration);
		REQUIRE(rate.advance() == expectedFrames);
		REQUIRE(rate.advance() == 0);
	};
	advance(999'999'999us, 0); // The delay before the first advance doesn't count.
	SUBCASE("advance()")
	{
		SUBCASE("1ms + 2ms + 3ms + 4ms + 5ms")
		{
			advance(1ms, 0);
			advance(2ms, 1);
			advance(3ms, 1);
			advance(4ms, 1);
			advance(5ms, 2);
		}
		SUBCASE("2999us + 1us + 1us + 2999us")
		{
			advance(2999us, 0);
			advance(1us, 1);
			advance(1us, 0);
			advance(2999us, 1);
		}
		SUBCASE("6999us + 2000us + 2us + 2999us")
		{
			advance(6999us, 2);
			advance(2000us, 0);
			advance(2us, 1);
			advance(2999us, 1);
		}
	}
	SUBCASE("reset()")
	{
		SUBCASE("after 2999us")
		{
			clock.advance(2999us);
			SUBCASE("without reset")
			{
				clock.advance(1us);
				REQUIRE(rate.advance() == 1);
			}
			SUBCASE("with reset")
			{
				rate.reset();
				clock.advance(1us);
				REQUIRE(rate.advance() == 0);
				clock.advance(2999us);
				REQUIRE(rate.advance() == 0);
				clock.advance(1us);
				REQUIRE(rate.advance() == 1);
			}
		}
		SUBCASE("after 3000us")
		{
			clock.advance(3000us);
			SUBCASE("without reset")
			{
				REQUIRE(rate.advance() == 1);
			}
			SUBCASE("with reset")
			{
				rate.reset();
				REQUIRE(rate.advance() == 0);
			}
			clock.advance(2999us);
			REQUIRE(rate.advance() == 0);
			clock.advance(1us);
			REQUIRE(rate.advance() == 1);
		}
		SUBCASE("after 3001us")
		{
			clock.advance(3001us);
			SUBCASE("without reset")
			{
				REQUIRE(rate.advance() == 1);
				clock.advance(2999us);
				REQUIRE(rate.advance() == 1);
			}
			SUBCASE("with reset")
			{
				rate.reset();
				REQUIRE(rate.advance() == 0);
				clock.advance(2999us);
				REQUIRE(rate.advance() == 0);
				clock.advance(1us);
				REQUIRE(rate.advance() == 1);
			}
		}
		REQUIRE(rate.advance() == 0);
	}
	SUBCASE("start()")
	{
		SUBCASE("after 2999us")
		{
			clock.advance(2999us);
			SUBCASE("without start")
			{
				clock.advance(1us);
				REQUIRE(rate.advance() == 1);
			}
			SUBCASE("with start")
			{
				rate.start();
				clock.advance(1us);
				REQUIRE(rate.advance() == 0);
				clock.advance(2998us);
				REQUIRE(rate.advance() == 0);
				clock.advance(1us);
				REQUIRE(rate.advance() == 1);
			}
		}
		SUBCASE("after 3000us")
		{
			clock.advance(3000us);
			SUBCASE("without start")
			{
				REQUIRE(rate.advance() == 1);
			}
			SUBCASE("with start")
			{
				rate.start();
				REQUIRE(rate.advance() == 0);
			}
			clock.advance(2999us);
			REQUIRE(rate.advance() == 0);
			clock.advance(1us);
			REQUIRE(rate.advance() == 1);
		}
		SUBCASE("after 3001us")
		{
			clock.advance(3001us);
			SUBCASE("without start")
			{
				REQUIRE(rate.advance() == 1);
				clock.advance(2999us);
				REQUIRE(rate.advance() == 1);
			}
			SUBCASE("with start")
			{
				rate.start();
				REQUIRE(rate.advance() == 0);
				clock.advance(2999us);
				REQUIRE(rate.advance() == 0);
				clock.advance(1us);
				REQUIRE(rate.advance() == 1);
			}
		}
		REQUIRE(rate.advance() == 0);
	}
}

TEST_CASE("VariableRate")
{
	using namespace std::literals::chrono_literals;
	using doctest::Approx;
	Clock clock{ 999'999'999us };
	seir::VariableRate<Clock> rate;
	REQUIRE(rate.time() == 0.f);
	const auto advance = [&clock, &rate](const Clock::duration& duration, auto&& expectedSeconds) {
		clock.advance(duration);
		auto period = rate.advance();
		REQUIRE(rate.time() == expectedSeconds);
		return period;
	};
	REQUIRE_FALSE(advance(999'999'999us, 0.f)); // The delay before the first advance doesn't count.
	SUBCASE("advance()")
	{
		SUBCASE("0s + 999'001us + 998us + 1us + 999ms + 1ms")
		{
			// Frame durations aren't rounded neither up nor down,
			// but peak frame duration metric is rounded up.
			REQUIRE_FALSE(advance(999'001us, Approx{ 0.999'001 }));
			REQUIRE_FALSE(advance(998us, Approx{ 0.999'999 }));
			auto period = advance(1us, 1.f);
			REQUIRE(period);
			CHECK(period->_frameCount == 3);
			CHECK(period->_averageFrameRate == 3.f);
			CHECK(period->_maxFrameDuration == 1000);

			// Peak frame duration metric doesn't have an extra millisecond.
			REQUIRE_FALSE(advance(999ms, Approx{ 1.999 }));
			period = advance(1ms, 2.f);
			REQUIRE(period);
			CHECK(period->_frameCount == 2);
			CHECK(period->_averageFrameRate == 2.f);
			CHECK(period->_maxFrameDuration == 999);
		}
		SUBCASE("250ms + 500ms + 750ms + 999'999us + 1us")
		{
			// The first period is longer than one second.
			REQUIRE_FALSE(advance(250ms, .25f));
			REQUIRE_FALSE(advance(500ms, .75f));
			auto period = advance(750ms, 1.5f);
			REQUIRE(period);
			CHECK(period->_frameCount == 3);
			CHECK(period->_averageFrameRate == 2.f);
			CHECK(period->_maxFrameDuration == 750);

			// The second period is not affected by the preceding long period.
			REQUIRE_FALSE(advance(999'999us, Approx{ 2.499'999 }));
			period = advance(1us, 2.5f);
			REQUIRE(period);
			CHECK(period->_frameCount == 2);
			CHECK(period->_averageFrameRate == 2.f);
			CHECK(period->_maxFrameDuration == 1000);
		}
	}
	// TODO: Add tests for start() and reset().
}
