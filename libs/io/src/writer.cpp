// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_io/writer.hpp>

#include <limits>

namespace seir
{
	bool Writer::seek(uint64_t offset) noexcept
	{
		if (offset > _size)
			return false;
		_offset = offset;
		return true;
	}

	bool Writer::reserve(uint64_t expectedBytes) noexcept
	{
		return expectedBytes <= std::numeric_limits<uint64_t>::max() - _offset
			&& reserveImpl(_offset + expectedBytes);
	}

	bool Writer::write(const void* data, size_t size) noexcept
	{
		if (!writeImpl(_offset, data, size))
			return false;
		_offset += size;
		if (_offset > _size)
			_size = _offset;
		return true;
	}
}
