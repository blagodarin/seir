// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "backend.hpp"

#include <seir_audio/player.hpp>
#include <seir_base/scope.hpp>

#include <AudioToolbox/AudioToolbox.h>

namespace
{
	void outputCallback(void*, OpaqueAudioQueue*, AudioQueueBuffer*)
	{
	}
}

namespace seir
{
	void runAudioBackend(AudioBackendCallbacks& callbacks, unsigned preferredSamplingRate)
	{
		const auto error = [&callbacks](const char* function, OSStatus status) {
			callbacks.onBackendError(function, static_cast<int>(status),
				[NSError errorWithDomain:NSOSStatusErrorDomain code:status userInfo:nil].description.UTF8String);
		};
		const AudioStreamBasicDescription format{
			.mSampleRate = static_cast<Float64>(preferredSamplingRate),
			.mFormatID = kAudioFormatLinearPCM,
			.mFormatFlags = kAudioFormatFlagIsFloat,
			.mBytesPerPacket = 8,
			.mFramesPerPacket = 1,
			.mBytesPerFrame = 8,
			.mChannelsPerFrame = 2,
			.mBitsPerChannel = 4,
			.mReserved = 0,
		};
		AudioQueueRef queue{};
		if (const auto status = ::AudioQueueNewOutput(&format, ::outputCallback, nullptr, nullptr, nullptr, 0, &queue))
			return error("AudioQueueNewOutput", status);
		SEIR_FINALLY{ [&]() noexcept {
			::AudioQueueStop(queue, true);
			::AudioQueueDispose(queue, false);
		} };
		// TODO: Implement.
	}
}
