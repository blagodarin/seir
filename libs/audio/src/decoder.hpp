// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

namespace seir
{
	class AudioDecoder;
	class AudioFormat;
	class Blob;
	template <class>
	class SharedPtr;
	template <class>
	class UniquePtr;

#if SEIR_AUDIO_AULOS
	UniquePtr<AudioDecoder> createAulosDecoder(const SharedPtr<Blob>&, const AudioFormat&);
#endif
#if SEIR_AUDIO_OGGVORBIS
	UniquePtr<AudioDecoder> createOggVorbisDecoder(const SharedPtr<Blob>&, const AudioFormat&);
#endif
#if SEIR_AUDIO_WAV
	UniquePtr<AudioDecoder> createWavDecoder(const SharedPtr<Blob>&, const AudioFormat&);
#endif
}
