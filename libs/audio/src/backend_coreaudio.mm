// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "backend.hpp"

#include <seir_audio/player.hpp>
#include <seir_base/scope.hpp>
#include "common.hpp"

#include <array>
#include <mutex>

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
		std::mutex _mutex;
		std::condition_variable _condition;
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

		AudioQueueBufferRef nextBuffer()
		{
			std::unique_lock lock{ _mutex };
			for (;;)
			{
				if (const auto i = std::find_if(_buffers.begin(), _buffers.end(),
						[this](auto buffer) { return buffer->mAudioDataByteSize == 0; });
					i != _buffers.end())
					return *i;
				_condition.wait(lock);
			}
		}

		static void outputCallback(void* userData, AudioQueueRef, AudioQueueBufferRef buffer)
		{
			auto& context = *static_cast<Context*>(userData);
			{
				std::lock_guard lock{ context._mutex };
				buffer->mAudioDataByteSize = 0;
			}
			context._condition.notify_one();
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
			callbacks.onBackendError(function, static_cast<int>(status),
				audioFileResultCode ? audioFileResultCode : [NSError errorWithDomain:NSOSStatusErrorDomain code:status userInfo:nil].description.UTF8String);
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
			if (const auto status = ::AudioQueueAllocateBuffer(context._queue, bufferBytes, &context._buffers[i]))
				return error("AudioQueueAllocateBuffer", status);
		if (const auto status = ::AudioQueueStart(context._queue, nullptr))
			return error("AudioQueueStart", status);
		constexpr auto bufferFrames = bufferBytes / kAudioFrameSize;
		callbacks.onBackendAvailable(preferredSamplingRate, bufferFrames);
		for (;;)
		{
			const auto buffer = context.nextBuffer();
			if (!callbacks.onBackendIdle())
				break;
			const auto writtenFrames = callbacks.onBackendRead(static_cast<float*>(buffer->mAudioData), bufferFrames);
			std::memset(static_cast<std::byte*>(buffer->mAudioData) + writtenFrames * kAudioFrameSize, 0, (bufferFrames - writtenFrames) * kAudioFrameSize);
			buffer->mAudioDataByteSize = bufferBytes;
			if (const auto status = ::AudioQueueEnqueueBuffer(context._queue, buffer, 0, nullptr))
				return error("AudioQueueEnqueueBuffer", status);
		}
	}
}
