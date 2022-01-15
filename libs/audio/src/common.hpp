// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/intrinsics.hpp>

#include <cstddef>
#include <numeric>

namespace seir
{
	constexpr size_t kAudioChannels = 2;
	constexpr size_t kAudioFrameSize = kAudioChannels * sizeof(float);
	constexpr size_t kAudioBlockAlignment = SEIR_INTRINSICS_SSE ? 16 : 1;
	constexpr size_t kAudioBlockSize = std::lcm(kAudioFrameSize, kAudioBlockAlignment);
	constexpr size_t kAudioFramesPerBlock = kAudioBlockSize / kAudioFrameSize;
	constexpr size_t kAudioResamplingFractionBits = 16;
	constexpr size_t kAudioResamplingFractionMask = (1 << kAudioResamplingFractionBits) - 1;
}
