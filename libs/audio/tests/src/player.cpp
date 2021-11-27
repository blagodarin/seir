// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_audio/decoder.hpp>
#include <seir_audio/player.hpp>

#include "common.hpp"

#include <condition_variable>
#include <cstring>
#include <mutex>
#include <thread>

#include <ostream>
#include <doctest/doctest.h>

namespace
{
	class SingleSourcePlayerTester
		: public seir::AudioDecoder
		, public seir::AudioCallbacks
	{
	public:
		explicit SingleSourcePlayerTester(const seir::AudioFormat& format) noexcept
			: _format{ format } {}

		void checkPostconditions() const
		{
			std::scoped_lock lock{ _mutex };
			if (!_skipPostconditions)
			{
				CHECK(_started);
				CHECK(_stopped);
				CHECK(_framesRemaining == 0);
			}
		}

		void waitForStop()
		{
			std::unique_lock lock{ _mutex };
			CHECK(_condition.wait_for(lock, std::chrono::seconds{ 5 }, [this] { return _stopped; }));
		}

	private:
		seir::AudioFormat format() const noexcept override
		{
			return _format;
		}

		size_t read(void* buffer, size_t maxFrames) noexcept override
		{
			CHECK(maxFrames > 0);
			std::scoped_lock lock{ _mutex };
			const auto result = std::min(_framesRemaining, maxFrames);
			if (result > 0)
			{
				std::memset(buffer, 0, result * _format.bytesPerFrame());
				_framesRemaining -= result;
			}
			MESSAGE(++_step, ") ", maxFrames, " -> ", result);
			return result;
		}

		bool seek(size_t) noexcept override
		{
			return false;
		}

		void onPlaybackError(seir::AudioError error) override
		{
			REQUIRE(error == seir::AudioError::NoDevice);
			MESSAGE("No audio playback device found");
			{
				std::scoped_lock lock{ _mutex };
				CHECK(!_started);
				_stopped = true;
				_skipPostconditions = true;
			}
			_condition.notify_one();
		}

		void onPlaybackError(std::string&& message) override
		{
			FAIL_CHECK(message);
			{
				std::scoped_lock lock{ _mutex };
				CHECK(!_stopped);
				_stopped = true;
				_skipPostconditions = true;
			}
			_condition.notify_one();
		}

		void onPlaybackStarted() override
		{
			std::scoped_lock lock{ _mutex };
			CHECK(!_started);
			_started = true;
		}

		void onPlaybackStopped() override
		{
			{
				std::scoped_lock lock{ _mutex };
				CHECK(_started);
				CHECK(!_framesRemaining);
				CHECK(!_stopped);
				_stopped = true;
			}
			_condition.notify_one();
		}

	private:
		const seir::AudioFormat _format;
		mutable std::mutex _mutex;
		std::condition_variable _condition;
		bool _started = false;
		unsigned _step = 0;
		size_t _framesRemaining = kTestFrames;
		bool _stopped = false;
		bool _skipPostconditions = false;
	};
}

TEST_CASE("player_single_source")
{
	seir::SharedPtr<SingleSourcePlayerTester> tester;
	SUBCASE("mono")
	{
		tester = seir::makeShared<SingleSourcePlayerTester>(seir::AudioFormat{ seir::AudioSampleType::f32, seir::AudioChannelLayout::Mono, kTestSamplingRate });
	}
	SUBCASE("stereo")
	{
		tester = seir::makeShared<SingleSourcePlayerTester>(seir::AudioFormat{ seir::AudioSampleType::f32, seir::AudioChannelLayout::Stereo, kTestSamplingRate });
	}
	{
		const auto player = seir::AudioPlayer::create(*tester, kTestSamplingRate);
		REQUIRE(player);
		player->play(seir::SharedPtr<seir::AudioDecoder>{ tester });
		tester->waitForStop();
	}
	tester->checkPostconditions();
}
