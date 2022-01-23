// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <atomic>

namespace seir
{
	class AppHelper
	{
	protected:
		AppHelper() noexcept = default;

		[[nodiscard]] bool beginQuit() noexcept
		{
			auto expected = State::Running;
			return _state.compare_exchange_strong(expected, State::Stopping, std::memory_order::acq_rel);
		}

		void endQuit() noexcept
		{
			_state.store(State::Stopped, std::memory_order::release);
		}

		[[nodiscard]] bool hasQuit() const noexcept
		{
			return _state.load(std::memory_order::acquire) == State::Stopped;
		}

	private:
		enum class State
		{
			Running,
			Stopping,
			Stopped,
		};

		std::atomic<State> _state{ State::Running };
	};
}
