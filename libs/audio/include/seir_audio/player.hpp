// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_audio/format.hpp>
#include <seir_base/shared_ptr.hpp>

#include <string>

namespace seir
{
	// Known audio playback errors.
	enum class AudioError
	{
		NoDevice, // No audio playback device found.
	};

	class AudioSource : public ReferenceCounter
	{
	public:
		virtual bool isStereo() const noexcept = 0;
		virtual size_t onRead(float* buffer, size_t maxFrames) noexcept = 0;
	};

	class AudioCallbacks
	{
	public:
		virtual ~AudioCallbacks() noexcept = default;

		virtual void onPlaybackError(AudioError) = 0;
		virtual void onPlaybackError(std::string&& message) = 0;
		virtual void onPlaybackStarted() = 0;
		virtual void onPlaybackStopped() = 0;
	};

	class AudioPlayer
	{
	public:
		[[nodiscard]] static UniquePtr<AudioPlayer> create(AudioCallbacks&, unsigned samplingRate);

		virtual ~AudioPlayer() noexcept = default;

		[[nodiscard]] virtual AudioFormat format() const noexcept = 0;
		virtual void play(const SharedPtr<AudioSource>&) = 0;
		virtual void stop() noexcept = 0;
	};
}
