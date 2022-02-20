// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// TODO: Move the #include to a common location.
#ifndef NDEBUG
#	ifdef _MSC_VER
#		pragma warning(push)
#		pragma warning(disable : 4061) // enumerator '...' in switch of enum '...' is not explicitly handled by a case label
#		pragma warning(disable : 4582) // constructor is not implicitly called
#	endif
#	include <fmt/core.h>
#	ifdef _MSC_VER
#		pragma warning(pop)
#	endif
#endif

namespace seir
{
	float clockTime() noexcept;
	void sleepFor(unsigned milliseconds) noexcept;
}
