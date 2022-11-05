// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_audio/format.hpp>
#include <seir_base/shared_ptr.hpp>

#include <cstddef>

namespace seir
{
	class Blob;

	// NOTE: Making it a member of AudioDecoder results in GCC/Clang compilation error,
	// see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=88165.
	struct AudioDecoderPreferences
	{
		AudioFormat format;
		bool loop = false;
	};

	//
	class AudioDecoder : public ReferenceCounter
	{
	public:
		//
		[[nodiscard]] static UniquePtr<AudioDecoder> create(SharedPtr<Blob>&&, const AudioDecoderPreferences& = {});

		virtual ~AudioDecoder() noexcept = default;

		// Returns the decoded audio format.
		[[nodiscard]] virtual AudioFormat format() const = 0;

		//
		[[nodiscard]] virtual size_t read(void* buffer, size_t maxFrames) = 0;

		// Restarts decoding from the specified offset.
		virtual bool seek(size_t frameOffset) = 0;

	private:
		struct
		{
			bool _finished = false;
			size_t _resamplingOffset = 0;
			float _resamplingBuffer[2]{};
		} _internal;
		friend class AudioMixer;
	};
}
