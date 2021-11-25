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

		// Returns the decoded audio format.
		[[nodiscard]] virtual AudioFormat format() const = 0;

		//
		[[nodiscard]] virtual size_t read(void* buffer, size_t maxFrames) = 0;

		// Restarts decoding from the specified offset.
		virtual bool seek(size_t frameOffset) = 0;
	};
}
