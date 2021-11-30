// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/macros.hpp>

#include <bit>
#include <cstdlib>
#include <cstring>
#include <new>

namespace seir
{
	class Allocator
	{
	public:
		[[nodiscard]] static void* allocate(size_t size)
		{
			if (const auto memory = std::malloc(size); memory) [[likely]]
				return memory;
			throw std::bad_alloc{};
		}

		static void deallocate(void* memory) noexcept
		{
			std::free(memory);
		}
	};

	template <size_t kAlignment>
	class AlignedAllocator
	{
	public:
		static_assert(std::has_single_bit(kAlignment));

		[[nodiscard]] static void* allocate(size_t size)
		{
			constexpr auto kAlignmentMask = kAlignment - 1;
			const auto alignedSize = (size + kAlignmentMask) & ~kAlignmentMask;
#ifdef _MSC_VER
			if (const auto memory = ::_aligned_malloc(alignedSize, kAlignment); memory) [[likely]]
#else
			if (const auto memory = std::aligned_alloc(kAlignment, alignedSize); memory) [[likely]]
#endif
				return memory;
			throw std::bad_alloc{};
		}

		static void deallocate(void* memory) noexcept
		{
#ifdef _MSC_VER
			::_aligned_free(memory);
#else
			std::free(memory);
#endif
		}
	};

	template <typename A>
	class CleanAllocator
	{
	public:
		[[nodiscard]] static void* allocate(size_t size)
		{
			const auto memory = A::allocate(size);
			std::memset(memory, 0, size);
			return memory;
		}

		static void deallocate(void* memory) noexcept
		{
			A::deallocate(memory);
		}
	};
}
