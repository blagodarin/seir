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

	struct Context
	{
		AudioQueueRef _queue{};
		std::array<AudioQueueBufferRef, 3> _buffers{};

		~Context() noexcept
		{
			for (auto i = _buffers.size(); i > 0;)
			{
				--i;
				if (_buffers[i])
					::AudioQueueFreeBuffer(_queue, _buffers[i]);
			}
			if (_queue)
			{
				::AudioQueueStop(_queue, true);
				::AudioQueueDispose(_queue, false);
			}
		}

		static void outputCallback(void*, AudioQueueRef, AudioQueueBufferRef)
		{
		}
	};
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
		Context context;
		if (const auto status = ::AudioQueueNewOutput(&format, Context::outputCallback, &context, nullptr, nullptr, 0, &context._queue))
			return error("AudioQueueNewOutput", status);
		constexpr auto bufferBytes = ::makeBufferBytes(0x1000);
		for (size_t i = 0; i < context._buffers.size(); ++i)
		{
			if (const auto status = ::AudioQueueAllocateBuffer(context._queue, bufferBytes, &context._buffers[i]))
				return error("AudioQueueAllocateBuffer", status);
			context._buffers[i]->mAudioDataByteSize = bufferBytes;
		}
		constexpr auto bufferFrames = bufferBytes / kAudioFrameSize;
		callbacks.onBackendAvailable(preferredSamplingRate, bufferFrames);
		for (size_t i = 0; callbacks.onBackendIdle(); i = (i + 1) % context._buffers.size())
		{
			const auto writtenFrames = callbacks.onBackendRead(static_cast<float*>(context._buffers[i]->mAudioData), bufferFrames);
			std::memset(static_cast<std::byte*>(context._buffers[i]->mAudioData) + writtenFrames * kAudioFrameSize, 0, (bufferFrames - writtenFrames) * kAudioFrameSize);
			if (const auto status = ::AudioQueueEnqueueBuffer(context._queue, context._buffers[i], 0, nullptr))
				return error("AudioQueueEnqueueBuffer", status);
			break; // TODO: Implement.
		}
	}
}
