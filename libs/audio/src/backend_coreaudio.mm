// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "backend.hpp"

#include <seir_audio/player.hpp>
#include <seir_base/scope.hpp>
#include "common.hpp"

#include <array>

#include <AudioToolbox/AudioToolbox.h>

namespace
{
	constexpr uint32_t makeBufferBytes(uint32_t minFrames) noexcept
	{
		const auto minBytes = minFrames * seir::kAudioFrameSize;
		return (minBytes + seir::kAudioBlockSize - 1) / seir::kAudioBlockSize * seir::kAudioBlockSize;
	}

	void outputCallback(void*, AudioQueueRef, AudioQueueBufferRef)
	{
	}
}

namespace seir
{
	void runAudioBackend(AudioBackendCallbacks& callbacks, unsigned preferredSamplingRate)
	{
		const auto error = [&callbacks](const char* function, OSStatus status) {
			const auto audioFileResultCode = [status]() -> const char* {
				switch (status)
				{
				case kAudioFileUnsupportedDataFormatError: return "kAudioFileUnsupportedDataFormatError";
				case kAudioQueueErr_BufferEmpty: return "kAudioQueueErr_BufferEmpty";
				}
				return nullptr;
			}();
			callbacks.onBackendError(function, static_cast<int>(status), audioFileResultCode
				? audioFileResultCode
				: [NSError errorWithDomain:NSOSStatusErrorDomain code:status userInfo:nil].description.UTF8String);
		};
		const AudioStreamBasicDescription format{
			.mSampleRate = static_cast<Float64>(preferredSamplingRate),
			.mFormatID = kAudioFormatLinearPCM,
			.mFormatFlags = kAudioFormatFlagIsFloat,
			.mBytesPerPacket = kAudioFrameSize,
			.mFramesPerPacket = 1,
			.mBytesPerFrame = kAudioFrameSize,
			.mChannelsPerFrame = kAudioChannels,
			.mBitsPerChannel = 32,
			.mReserved = 0,
		};
		AudioQueueRef queue{};
		if (const auto status = ::AudioQueueNewOutput(&format, ::outputCallback, nullptr, nullptr, nullptr, 0, &queue))
			return error("AudioQueueNewOutput", status);
		SEIR_FINALLY{ [&]() noexcept {
			::AudioQueueStop(queue, true);
			::AudioQueueDispose(queue, false);
		} };
		std::array<AudioQueueBufferRef, 3> buffers{};
		constexpr auto bufferBytes = ::makeBufferBytes(0x1000);
		if (const auto status = ::AudioQueueAllocateBuffer(queue, bufferBytes, &buffers[0]))
			return error("AudioQueueAllocateBuffer", status);
		SEIR_FINALLY{ [&]() noexcept { ::AudioQueueFreeBuffer(queue, buffers[0]); } };
		buffers[0]->mAudioDataByteSize = bufferBytes;
		if (const auto status = ::AudioQueueAllocateBuffer(queue, bufferBytes, &buffers[1]))
			return error("AudioQueueAllocateBuffer", status);
		SEIR_FINALLY{ [&]() noexcept { ::AudioQueueFreeBuffer(queue, buffers[1]); } };
		buffers[1]->mAudioDataByteSize = bufferBytes;
		if (const auto status = ::AudioQueueAllocateBuffer(queue, bufferBytes, &buffers[2]))
			return error("AudioQueueAllocateBuffer", status);
		SEIR_FINALLY{ [&]() noexcept { ::AudioQueueFreeBuffer(queue, buffers[2]); } };
		buffers[2]->mAudioDataByteSize = bufferBytes;
		constexpr auto bufferFrames = bufferBytes / kAudioFrameSize;
		callbacks.onBackendAvailable(preferredSamplingRate, bufferFrames);
		for (size_t i = 0; callbacks.onBackendIdle(); i = (i + 1) % buffers.size())
		{
			const auto writtenFrames = callbacks.onBackendRead(static_cast<float*>(buffers[i]->mAudioData), bufferFrames);
			std::memset(static_cast<std::byte*>(buffers[i]->mAudioData) + writtenFrames * kAudioFrameSize, 0, (bufferFrames - writtenFrames) * kAudioFrameSize);
			if (const auto status = ::AudioQueueEnqueueBuffer(queue, buffers[i], 0, nullptr))
				return error("AudioQueueEnqueueBuffer", status);
			break; // TODO: Implement.
		}
	}
}
