// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_audio/format.hpp>
#include <seir_base/shared_ptr.hpp>

#include <string>

namespace seir
{
	class AudioDecoder;

	// Known audio playback errors.
	enum class AudioError
	{
		NoDevice, // No audio playback device has been found.
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
		[[nodiscard]] static UniquePtr<AudioPlayer> create(AudioCallbacks&, unsigned preferredSamplingRate = AudioFormat::kMaxSamplingRate);

		virtual ~AudioPlayer() noexcept = default;

		// Plays audio from the specified decoder. The audio is always played from the beginning.
		// NOTE: The player uses the decoder asynchronously, even after it has been stopped.
		virtual void play(const SharedPtr<AudioDecoder>&) = 0;

		// Stops playing audio from the specified decoder.
		virtual void stop(const SharedPtr<const AudioDecoder>&) noexcept = 0;

		// Stops all currently playing audio.
		virtual void stopAll() noexcept = 0;
	};
}
