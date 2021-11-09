// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <memory>
#include <string>

namespace seir
{
	// Known audio playback errors.
	enum class AudioError
	{
		NoDevice, // No audio playback device found.
	};

	class AudioSource
	{
	public:
		virtual ~AudioSource() noexcept = default;

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
		[[nodiscard]] static std::unique_ptr<AudioPlayer> create(AudioCallbacks&, unsigned samplingRate);

		virtual ~AudioPlayer() noexcept = default;

		virtual void play(const std::shared_ptr<AudioSource>&) = 0;
		[[nodiscard]] virtual unsigned samplingRate() const noexcept = 0;
		virtual void stop() noexcept = 0;
	};
}
