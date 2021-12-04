// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/shared_ptr.hpp>

#include <cstddef>

namespace seir
{
	class AudioFormat;
	class Blob;

	class AudioDecoder : public ReferenceCounter
	{
	public:
		[[nodiscard]] static UniquePtr<AudioDecoder> create(const SharedPtr<Blob>&, const AudioFormat& preferredFormat);

		// Wraps custom audio decoder to be used by audio player.
		// TODO: Come up with a way to make this less error-prone.
		[[nodiscard]] static UniquePtr<AudioDecoder> custom(const SharedPtr<AudioDecoder>&);

		// Returns the decoded audio format.
		[[nodiscard]] virtual AudioFormat format() const = 0;

		//
		[[nodiscard]] virtual size_t read(void* buffer, size_t maxFrames) = 0;

		// Restarts decoding from the specified offset.
		virtual bool seek(size_t frameOffset) = 0;
	};
}
