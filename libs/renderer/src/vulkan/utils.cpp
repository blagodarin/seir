// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "utils.hpp"

#include <thread> // This header is very slow to compile, so it has been moved away from frequently-changing sources.

namespace seir
{
	float clockTime() noexcept
	{
		static auto startTime = std::chrono::steady_clock::now();
		return std::chrono::duration_cast<std::chrono::duration<float, std::chrono::seconds::period>>(std::chrono::steady_clock::now() - startTime).count();
	}

	void sleepFor(unsigned milliseconds) noexcept
	{
		std::this_thread::sleep_for(std::chrono::milliseconds{ milliseconds });
	}
}
