// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/unique_ptr.hpp>

#include <cstddef>

namespace seir
{
	// Supported compression algorithms.
	enum class Compression
	{
		None, // Special value to specify no compression.
		Zlib,
		Zstd,
	};

	//
	enum class CompressionLevel
	{
		None,
		Minimum,
		Default,
		Maximum,
	};

	// Data compression interface.
	class Compressor
	{
	public:
		//
		static UniquePtr<Compressor> create(Compression);

		virtual ~Compressor() noexcept = default;

		// Prepares for compression. Must be called before every compress() call.
		// This is a separate function (and not a level parameter in other functions)
		// because some compression algorithms (namely zlib) require full initialization
		// before compressed data size estimation.
		[[nodiscard]] virtual bool prepare(CompressionLevel) noexcept = 0;

		// Returns the maximum compressed data size for uncompressed data of the specified size.
		[[nodiscard]] virtual size_t maxCompressedSize(size_t uncompressedSize) const noexcept = 0;

		// Compresses the input data into the output buffer.
		// Returns the actual compressed data size.
		// The compressor must be prepared again before the next call to compress().
		[[nodiscard]] virtual size_t compress(void* dst, size_t dstCapacity, const void* src, size_t srcSize) noexcept = 0;
	};

	// Data decompression interface.
	class Decompressor
	{
	public:
		//
		static UniquePtr<Decompressor> create(Compression);

		virtual ~Decompressor() noexcept = default;
		[[nodiscard]] virtual bool decompress(void* dst, size_t dstCapacity, const void* src, size_t srcSize) noexcept = 0;
	};
}
