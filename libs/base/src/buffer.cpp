// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_base/buffer.hpp>

#include <seir_base/allocator.hpp>
#include <seir_base/intrinsics.hpp>

#include <cstring>

namespace
{
	using BufferAllocator =
#if SEIR_INTRINSICS_SSE
		seir::AlignedAllocator<16>
#else
		seir::Allocator
#endif
		;
}

namespace seir
{
	Buffer::~Buffer() noexcept
	{
		BufferAllocator::deallocate(_data);
	}

	Buffer::Buffer(size_t capacity)
		: _data{ static_cast<std::byte*>(BufferAllocator::allocate(capacity)) }
		, _capacity{ capacity }
	{
	}

	bool Buffer::tryReserve(size_t totalCapacity, size_t preservedCapacity) noexcept
	{
		if (totalCapacity <= _capacity)
			return true;
		const auto data = BufferAllocator::tryAllocate(totalCapacity);
		if (!data) [[unlikely]]
			return false;
		if (preservedCapacity > 0)
			std::memcpy(data, _data, (preservedCapacity < _capacity ? preservedCapacity : _capacity));
		BufferAllocator::deallocate(_data);
		_data = static_cast<std::byte*>(data);
		_capacity = totalCapacity;
		return true;
	}
}
