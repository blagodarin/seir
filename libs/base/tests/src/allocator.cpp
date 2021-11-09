// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_base/allocator.hpp>

#include <algorithm>
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

	using CleanAllocator = TestAllocator<seir::CleanAllocator<seir::Allocator>>;
	using CleanAllocatorPtr = std::unique_ptr<void, CleanAllocator>;
}

TEST_CASE("Allocator::allocate(1)")
{
	AllocatorPtr pointer{ seir::Allocator::allocate(1) };
	CHECK(pointer);
	const auto misalignment = reinterpret_cast<uintptr_t>(pointer.get()) % kAlignment;
	MESSAGE("Default allocator misalignment: ", misalignment, " % ", kAlignment);
	CHECK(misalignment != 0);
}

TEST_CASE("AlignedAllocator::allocate(1)")
{
	AlignedAllocatorPtr pointer{ AlignedAllocator::allocate(1) };
	CHECK(pointer);
	CHECK(reinterpret_cast<uintptr_t>(pointer.get()) % kAlignment == 0);
}

TEST_CASE("CleanAllocator::allocate()")
{
	constexpr size_t size = 512;
	CleanAllocatorPtr pointer{ CleanAllocator::allocate(size) };
	REQUIRE(pointer);
	const auto data = reinterpret_cast<std::byte*>(pointer.get());
	CHECK(data + size == std::find_if(data, data + size, [](std::byte byte) { return std::to_integer<int>(byte) != 0; }));
}

// std::malloc doesn't return nullptr in ASAN-less Clang builds.
#if !defined(__clang__)

// GCC issues a warning if the allocation size is known at compile time and exceeds this value.
constexpr auto kMaxSize = static_cast<size_t>(std::numeric_limits<std::make_signed_t<size_t>>::max());

TEST_CASE("Allocator::allocate(SIZE_MAX)")
{
	CHECK_THROWS_AS(AllocatorPtr pointer{ seir::Allocator::allocate(kMaxSize) }, std::bad_alloc);
}

TEST_CASE("AlignedAllocator::allocate(SIZE_MAX)")
{
	CHECK_THROWS_AS(AlignedAllocatorPtr pointer{ AlignedAllocator::allocate(kMaxSize - kMaxSize % kAlignment) }, std::bad_alloc);
}

#endif
