// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_data/compression.hpp>

namespace seir
{
#if SEIR_COMPRESSION_ZLIB
	UniquePtr<Compressor> createZlibCompressor();
	UniquePtr<Decompressor> createZlibDecompressor();
#endif
}
