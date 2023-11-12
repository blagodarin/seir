// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "compression.hpp"

#include <seir_base/pointer.hpp>

#include <cassert>

#include <zstd.h>

namespace
{
	class ZstdCompressor final : public seir::Compressor
	{
	public:
		[[nodiscard]] bool prepare(seir::CompressionLevel level) noexcept override
		{
			switch (level)
			{
			case seir::CompressionLevel::None: _level = 1; break;    // There is no zero compression level in zstd.
			case seir::CompressionLevel::Minimum: _level = 1; break; // Negative levels (ZSTD_minCLevel() to -1) ara faster but have impractical compression ratios.
			case seir::CompressionLevel::Default: _level = ::ZSTD_defaultCLevel(); break;
			case seir::CompressionLevel::Maximum: _level = ::ZSTD_maxCLevel(); break;
			}
			return true;
		}

		[[nodiscard]] size_t maxCompressedSize(size_t uncompressedSize) const noexcept override
		{
			return ::ZSTD_compressBound(uncompressedSize);
		}

		[[nodiscard]] size_t compress(void* dst, size_t dstCapacity, const void* src, size_t srcSize) noexcept override
		{
			const auto result = ::ZSTD_compressCCtx(_context, dst, dstCapacity, src, srcSize, _level);
			return ::ZSTD_isError(result) ? 0 : result;
		}

	private:
		seir::CPtr<::ZSTD_CCtx, ::ZSTD_freeCCtx> _context{ ::ZSTD_createCCtx() };
		int _level = 0;
	};

	class ZstdDecompressor final : public seir::Decompressor
	{
	public:
		[[nodiscard]] bool decompress(void* dst, size_t dstCapacity, const void* src, size_t srcSize) noexcept override
		{
			const auto result = ::ZSTD_decompressDCtx(_context, dst, dstCapacity, src, srcSize);
			return !::ZSTD_isError(result) && result == dstCapacity;
		}

	private:
		seir::CPtr<::ZSTD_DCtx, ::ZSTD_freeDCtx> _context{ ::ZSTD_createDCtx() };
	};
}

namespace seir
{
	UniquePtr<Compressor> createZstdCompressor()
	{
		return makeUnique<Compressor, ZstdCompressor>();
	}

	UniquePtr<Decompressor> createZstdDecompressor()
	{
		return makeUnique<Decompressor, ZstdDecompressor>();
	}
}
