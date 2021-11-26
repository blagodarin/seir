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

	UniquePtr<AudioDecoder> createAulosDecoder(const SharedPtr<Blob>&, const AudioFormat&);
}
