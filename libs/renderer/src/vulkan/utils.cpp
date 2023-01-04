// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "utils.hpp"

#include <thread> // This header is very slow to compile, so it has been moved away from frequently-changing sources.

namespace seir
{
	void sleepFor(unsigned milliseconds) noexcept
	{
		std::this_thread::sleep_for(std::chrono::milliseconds{ milliseconds });
	}
}
