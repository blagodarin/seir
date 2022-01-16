// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_base/allocator.hpp>

namespace seir
{
	void AlignedAllocatorImpl::deallocate(void* memory) noexcept
	{
#ifdef _MSC_VER
		::_aligned_free(memory);
#else
		std::free(memory);
#endif
	}

	void* AlignedAllocatorImpl::tryAllocate(size_t& size, size_t alignment) noexcept
	{
		const auto alignmentMask = alignment - 1;
		const auto alignedSize = (size + alignmentMask) & ~alignmentMask;
#ifdef _MSC_VER
		const auto memory = ::_aligned_malloc(alignedSize, alignment);
		if (!memory) [[unlikely]]
			return nullptr;
		size = ::_aligned_msize(memory, alignment, 0);
		return memory;
#else
		const auto memory = std::aligned_alloc(alignment, alignedSize);
		if (!memory) [[unlikely]]
			return nullptr;
		size = alignedSize;
		return memory;
#endif
	}
}
