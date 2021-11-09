// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#if defined(_M_AMD64) || defined(_M_IX86) || defined(__amd64) || defined(__i386)
#	ifdef _MSC_VER
#		include <intrin.h>
#	else
#		include <x86intrin.h>
#	endif
#	define SEIR_INTRINSICS_SSE 1
#else
#	define SEIR_INTRINSICS_SSE 0
#endif
