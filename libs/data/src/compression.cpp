// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "compression.hpp"

#include <cassert>

namespace seir
{
	UniquePtr<Compressor> Compressor::create(Compression compression)
	{
		switch (compression)
		{
		case Compression::None:
			break;
		case Compression::Zlib:
#if SEIR_COMPRESSION_ZLIB
			return createZlibCompressor();
#else
			break;
#endif
		}
		assert(false);
		return {};
	}

	UniquePtr<Decompressor> Decompressor::create(Compression compression)
	{
		switch (compression)
		{
		case Compression::None:
			break;
		case Compression::Zlib:
#if SEIR_COMPRESSION_ZLIB
			return createZlibDecompressor();
#else
			break;
#endif
		}
		assert(false);
		return {};
	}
}
