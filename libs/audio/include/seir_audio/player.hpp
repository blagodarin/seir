// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/shared_ptr.hpp>

#include <string>

namespace seir
{
	class AudioDecoder;

	// Known audio playback errors.
	enum class AudioError
	{
		NoDevice, // No audio playback device found.
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
		[[nodiscard]] static UniquePtr<AudioPlayer> create(AudioCallbacks&, unsigned preferredSamplingRate);

		virtual ~AudioPlayer() noexcept = default;

		//
		virtual void play(const SharedPtr<AudioDecoder>&) = 0;

		//
		virtual void stop(const SharedPtr<const AudioDecoder>&) noexcept = 0;

		//
		virtual void stopAll() noexcept = 0;
	};
}
