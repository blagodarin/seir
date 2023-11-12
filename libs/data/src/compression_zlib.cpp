// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "compression.hpp"

#include <cassert>
#include <limits>
#include <optional>

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
			if (_level)
			{
				if (::deflateReset(&_stream) != Z_OK)
					return false;
				if (_level == level)
					return true;
			}
			int levelValue = Z_NO_COMPRESSION;
			switch (level)
			{
			case seir::CompressionLevel::None: break;
			case seir::CompressionLevel::Minimum: levelValue = Z_BEST_SPEED; break;
			case seir::CompressionLevel::Default: levelValue = Z_DEFAULT_COMPRESSION; break;
			case seir::CompressionLevel::Maximum: levelValue = Z_BEST_COMPRESSION; break;
			}
			if (_level
					? ::deflateParams(&_stream, levelValue, Z_DEFAULT_STRATEGY) != Z_OK
						|| ::deflateReset(&_stream) != Z_OK
					: deflateInit(&_stream, levelValue) != Z_OK)
				return false;
			_level = level;
			return true;
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
			if constexpr (constexpr auto maxSize = std::numeric_limits<uInt>::max(); maxSize < std::numeric_limits<size_t>::max())
			{
				if (srcSize > maxSize)
					return 0;
				if (dstCapacity > maxSize)
					dstCapacity = maxSize;
			}
			_stream.next_in = static_cast<const Bytef*>(src);
			_stream.avail_in = static_cast<uInt>(srcSize);
			_stream.next_out = static_cast<Bytef*>(dst);
			_stream.avail_out = static_cast<uInt>(dstCapacity);
			return ::deflate(&_stream, Z_FINISH) == Z_STREAM_END ? dstCapacity - _stream.avail_out : 0;
		}

	private:
		z_stream _stream{};
		std::optional<seir::CompressionLevel> _level;
	};

	class ZlibDecompressor final : public seir::Decompressor
	{
	public:
		~ZlibDecompressor() noexcept override
		{
			::inflateEnd(&_stream);
		}

		[[nodiscard]] bool decompress(void* dst, size_t dstCapacity, const void* src, size_t srcSize) noexcept override
		{
			if constexpr (constexpr auto maxSize = std::numeric_limits<uInt>::max(); maxSize < std::numeric_limits<size_t>::max())
			{
				if (srcSize > maxSize)
					return false;
				if (dstCapacity > maxSize)
					dstCapacity = maxSize;
			}
			if (!_initialized)
			{
				if (inflateInit(&_stream) != Z_OK)
					return false;
				_initialized = true;
			}
			else if (::inflateReset(&_stream) != Z_OK)
				return false;
			_stream.next_in = static_cast<const Bytef*>(src);
			_stream.avail_in = static_cast<uInt>(srcSize);
			_stream.next_out = static_cast<Bytef*>(dst);
			_stream.avail_out = static_cast<uInt>(dstCapacity);
			return ::inflate(&_stream, Z_FINISH) == Z_STREAM_END;
		}

	private:
		z_stream _stream{};
		bool _initialized = false;
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
