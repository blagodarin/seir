// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_audio/format.hpp>

#include <cstddef>
#include <utility>

namespace seir
{
	class Blob;
	template <class>
	class SharedPtr;
	template <class>
	class UniquePtr;

	class AudioDecoder
	{
	public:
		[[nodiscard]] static UniquePtr<AudioDecoder> create(const SharedPtr<Blob>&, const AudioFormat& preferredFormat);

		virtual ~AudioDecoder() noexcept = default;

		// Returns audio format.
		[[nodiscard]] constexpr AudioFormat format() const noexcept { return _format; }

		//
		virtual std::pair<const void*, size_t> decode(void* buffer, size_t maxFrames) = 0;

		// Restarts decoding from the beginning.
		virtual void restart() = 0;

	protected:
		const AudioFormat _format;
		constexpr explicit AudioDecoder(const AudioFormat& format) noexcept
			: _format{ format } {}
	};
}
