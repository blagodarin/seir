// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_compression/compression.hpp>

namespace seir
{
#if SEIR_COMPRESSION_ZLIB
	UniquePtr<Compressor> createZlibCompressor();
	UniquePtr<Decompressor> createZlibDecompressor();
#endif
#if SEIR_COMPRESSION_ZSTD
	UniquePtr<Compressor> createZstdCompressor();
	UniquePtr<Decompressor> createZstdDecompressor();
#endif
}
