// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_audio/processing.hpp>

#include <numeric>

namespace seir
{
	constexpr size_t kAudioChannels = 2;
	constexpr auto kAudioFrameSize = kAudioChannels * sizeof(float);
	constexpr auto kAudioFramesPerBlock = std::lcm(kAudioFrameSize, kAudioBlockAlignment) / kAudioFrameSize;
}
