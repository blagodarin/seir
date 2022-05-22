// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <bit>
#include <cstdlib>
#include <new>

namespace seir
{
	//
	class Allocator
	{
	public:
		//
		[[nodiscard]] static void* allocate(size_t& size);

		//
		static void deallocate(void* memory) noexcept { std::free(memory); }

		//
		[[nodiscard]] static void* tryAllocate(size_t& size) noexcept { return std::malloc(size); } // cppcheck-suppress[constParameter]
	};

	class AlignedAllocatorImpl
	{
		template <size_t>
		friend class AlignedAllocator;
		static void deallocate(void* memory) noexcept;
		[[nodiscard]] static void* tryAllocate(size_t& size, size_t alignment) noexcept;
	};

	//
	template <size_t kAlignment>
	class AlignedAllocator
	{
	public:
		static_assert(std::has_single_bit(kAlignment));

		//
		[[nodiscard]] static void* allocate(size_t& size);

		//
		static void deallocate(void* memory) noexcept { AlignedAllocatorImpl::deallocate(memory); }

		//
		[[nodiscard]] static void* tryAllocate(size_t& size) noexcept { return AlignedAllocatorImpl::tryAllocate(size, kAlignment); }
	};
}

inline void* seir::Allocator::allocate(size_t& size)
{
	if (const auto memory = tryAllocate(size); memory) [[likely]]
		return memory;
	throw std::bad_alloc{};
}

template <size_t kAlignment>
void* seir::AlignedAllocator<kAlignment>::allocate(size_t& size)
{
	if (const auto memory = tryAllocate(size); memory) [[likely]]
		return memory;
	throw std::bad_alloc{};
}
