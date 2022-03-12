// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#ifndef NDEBUG
#	include <seir_3rdparty/fmt.hpp>
#endif

namespace seir
{
	float clockTime() noexcept;
	void sleepFor(unsigned milliseconds) noexcept;
}
