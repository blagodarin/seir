// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_audio/processing.hpp>

#include <numeric>
#include <string>

namespace seir
{
	enum class AudioError;

	constexpr auto kAudioBackendChannels = 2u;
	constexpr auto kAudioBackendFrameBytes = kAudioBackendChannels * sizeof(float);
	constexpr auto kAudioBackendFrameAlignment = std::lcm(kAudioAlignment, kAudioBackendFrameBytes) / kAudioBackendFrameBytes;

	class AudioBackendCallbacks
	{
	public:
		virtual ~AudioBackendCallbacks() noexcept = default;

		virtual void onBackendAvailable(unsigned samplingRate, size_t maxReadFrames) = 0;
		virtual void onBackendError(AudioError) = 0;
		virtual void onBackendError(const char* function, int code, const std::string& description) = 0;
		virtual bool onBackendIdle() = 0;
		virtual size_t onBackendRead(float* output, size_t maxFrames) noexcept = 0;
	};

	void runAudioBackend(AudioBackendCallbacks&, unsigned preferredSamplingRate);
}
