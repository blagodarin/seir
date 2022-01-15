// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/allocator.hpp>
#include <seir_base/pointer.hpp>

#include <cstring>

// TODO: Buffers should probably have a single specialized allocator optimized for large-ish memory blocks.

namespace seir
{
	//
	template <class A = Allocator>
	class Buffer
	{
	public:
		constexpr Buffer() noexcept = default;
		Buffer(const Buffer&) = delete;
		constexpr Buffer(Buffer&& other) noexcept
			: _data{ std::move(other._data) }, _capacity{ other._capacity } { other._capacity = 0; }
		Buffer& operator=(Buffer&) = delete;
		constexpr Buffer& operator=(Buffer&&) noexcept;
		~Buffer() noexcept = default;

		//
		explicit Buffer(size_t capacity)
			: _data{ static_cast<std::byte*>(A::allocate(capacity)) }, _capacity{ capacity } {}

		//
		[[nodiscard]] constexpr size_t capacity() const noexcept { return _capacity; }

		//
		[[nodiscard]] constexpr std::byte* data() noexcept { return _data; }

		//
		[[nodiscard]] constexpr const std::byte* data() const noexcept { return _data; }

		//
		void reserve(size_t totalCapacity, size_t preservedCapacity);

		//
		[[nodiscard]] bool tryReserve(size_t totalCapacity, size_t preservedCapacity) noexcept;

		friend constexpr void swap(Buffer& first, Buffer& second) noexcept
		{
			using std::swap;
			swap(first._data, second._data);
			swap(first._capacity, second._capacity);
		}

	private:
		CPtr<std::byte, A::deallocate> _data;
		size_t _capacity = 0;
	};
}

template <class A>
constexpr seir::Buffer<A>& seir::Buffer<A>::operator=(Buffer&& other) noexcept
{
	_data = std::move(other._data);
	_capacity = other._capacity;
	return *this;
}

template <class A>
void seir::Buffer<A>::reserve(size_t totalCapacity, size_t preservedCapacity)
{
	if (!tryReserve(totalCapacity, preservedCapacity)) [[unlikely]]
		throw std::bad_alloc{};
}

template <class A>
bool seir::Buffer<A>::tryReserve(size_t totalCapacity, size_t preservedCapacity) noexcept
{
	if (totalCapacity <= _capacity)
		return true;
	decltype(_data) newData{ static_cast<std::byte*>(A::tryAllocate(totalCapacity)) };
	if (!newData) [[unlikely]]
		return false;
	if (preservedCapacity > 0)
		std::memcpy(newData, _data, (preservedCapacity < _capacity ? preservedCapacity : _capacity));
	_data = std::move(newData);
	_capacity = totalCapacity;
	return true;
}
