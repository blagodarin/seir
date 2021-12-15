// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "compression.hpp"

#include <cassert>

#include <zlib.h>

namespace
{
	class ZlibCompressor final : public seir::Compressor
	{
	public:
		~ZlibCompressor() noexcept override
		{
			::deflateEnd(&_stream);
		}

		[[nodiscard]] bool prepare(seir::CompressionLevel level) noexcept override
		{
			// TODO: Use a previously-prepared compressor without unnecessary deallocations (in deflateEnd)
			// and allocations (in deflateInit). Using deflateParams (and maybe deflateReset) may help.
			int levelValue = Z_NO_COMPRESSION;
			switch (level)
			{
			case seir::CompressionLevel::BestSpeed: levelValue = Z_BEST_SPEED; break;
			case seir::CompressionLevel::BestCompression: levelValue = Z_BEST_COMPRESSION; break;
			}
			::deflateEnd(&_stream);
			return deflateInit(&_stream, levelValue) == Z_OK;
		}

		[[nodiscard]] size_t maxCompressedSize(size_t uncompressedSize) const noexcept override
		{
			// deflateBound DOES return some valid (but suboptimal) bound
			// even for uninitialized streams, so no hard checks here.
			assert(_stream.state);
			return ::deflateBound(const_cast<z_stream*>(&_stream), static_cast<uLong>(uncompressedSize));
		}

		[[nodiscard]] size_t compress(void* dst, size_t dstCapacity, const void* src, size_t srcSize) noexcept override
		{
			_stream.next_in = static_cast<const Bytef*>(src);
			_stream.avail_in = static_cast<uInt>(srcSize); // TODO: Check for overflow.
			_stream.next_out = static_cast<Bytef*>(dst);
			_stream.avail_out = static_cast<uInt>(dstCapacity); // TODO: Check for overflow.
			const auto status = ::deflate(&_stream, Z_FINISH);
			::deflateEnd(&_stream);
			return status == Z_STREAM_END ? dstCapacity - _stream.avail_out : 0;
		}

	private:
		z_stream _stream{};
	};

	class ZlibDecompressor final : public seir::Decompressor
	{
	public:
		[[nodiscard]] bool decompress(void* dst, size_t dstCapacity, const void* src, size_t srcSize) noexcept override
		{
			if (inflateInit(&_stream) != Z_OK)
				return false;
			_stream.next_in = static_cast<const Bytef*>(src);
			_stream.avail_in = static_cast<uInt>(srcSize); // TODO: Check for overflow.
			_stream.next_out = static_cast<Bytef*>(dst);
			_stream.avail_out = static_cast<uInt>(dstCapacity); // TODO: Check for overflow.
			const auto status = ::inflate(&_stream, Z_FINISH);
			::inflateEnd(&_stream);
			return status == Z_STREAM_END;
		}

	private:
		z_stream _stream{};
	};
}

namespace seir
{
	UniquePtr<Compressor> createZlibCompressor()
	{
		return makeUnique<Compressor, ZlibCompressor>();
	}

	UniquePtr<Decompressor> createZlibDecompressor()
	{
		return makeUnique<Decompressor, ZlibDecompressor>();
	}
}
