// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_data/buffer_writer.hpp>

#include <seir_base/buffer.hpp>

#include <cstring>
#include <limits>

namespace seir
{
	bool BufferWriter::flush() noexcept
	{
		return true;
	}

	bool BufferWriter::reserveImpl(uint64_t capacity) noexcept
	{
		if constexpr (sizeof(uint64_t) > sizeof(size_t))
			if (capacity > std::numeric_limits<size_t>::max())
				return false;
		return _buffer.tryReserve(static_cast<size_t>(capacity), static_cast<size_t>(size()));
	}

	bool BufferWriter::writeImpl(uint64_t offset64, const void* data, size_t size) noexcept
	{
		if constexpr (sizeof(uint64_t) > sizeof(size_t))
			if (offset64 > std::numeric_limits<size_t>::max())
				return false;
		const auto offset = static_cast<size_t>(offset64);
		if (size > std::numeric_limits<size_t>::max() - offset)
			return false;
		const auto requiredCapacity = offset + size;
		if (requiredCapacity > _buffer.capacity())
		{
			const auto grownCapacity = _buffer.capacity() + _buffer.capacity() / 2;
			if (!_buffer.tryReserve(requiredCapacity > grownCapacity ? requiredCapacity : grownCapacity, static_cast<size_t>(Writer::size())))
				return false;
		}
		std::memcpy(_buffer.data() + offset, data, size);
		if (_bufferBytes && *_bufferBytes < requiredCapacity)
			*_bufferBytes = requiredCapacity;
		return true;
	}
}
