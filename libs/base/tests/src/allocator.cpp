// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_base/allocator.hpp>

#include <cstddef>
#include <limits>
#include <memory>
#include <type_traits>

#include <doctest/doctest.h>

namespace
{
	template <typename A>
	struct TestAllocator : public A
	{
		void operator()(void* pointer) noexcept { A::deallocate(pointer); }
	};

	using Allocator = TestAllocator<seir::Allocator>;
	using AllocatorPtr = std::unique_ptr<void, Allocator>;

	constexpr size_t kAlignment = 16'384;
	using AlignedAllocator = TestAllocator<seir::AlignedAllocator<kAlignment>>;
	using AlignedAllocatorPtr = std::unique_ptr<void, AlignedAllocator>;
}

TEST_CASE("Allocator::allocate(1)")
{
	size_t size = 1;
	AllocatorPtr pointer{ seir::Allocator::allocate(size) };
	CHECK(pointer);
	const auto misalignment = reinterpret_cast<uintptr_t>(pointer.get()) % kAlignment;
	MESSAGE("Default allocator misalignment: ", misalignment, " % ", kAlignment);
	CHECK(misalignment != 0);
}

TEST_CASE("AlignedAllocator::allocate(1)")
{
	size_t size = 1;
	AlignedAllocatorPtr pointer{ AlignedAllocator::allocate(size) };
	CHECK(pointer);
	CHECK(reinterpret_cast<uintptr_t>(pointer.get()) % kAlignment == 0);
}

// std::malloc doesn't return nullptr in ASAN-less Clang builds.
#if !defined(__clang__)

// GCC issues a warning if the allocation size is known at compile time and exceeds this value.
constexpr auto kMaxSize = static_cast<size_t>(std::numeric_limits<std::make_signed_t<size_t>>::max());

TEST_CASE("Allocator::allocate(SIZE_MAX)")
{
	auto size = kMaxSize;
	CHECK_THROWS_AS(AllocatorPtr pointer{ seir::Allocator::allocate(size) }, std::bad_alloc);
}

TEST_CASE("AlignedAllocator::allocate(SIZE_MAX)")
{
	auto size = kMaxSize - kMaxSize % kAlignment;
	CHECK_THROWS_AS(AlignedAllocatorPtr pointer{ AlignedAllocator::allocate(size) }, std::bad_alloc);
}

#endif
