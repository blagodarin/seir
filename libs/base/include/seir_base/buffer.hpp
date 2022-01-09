// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/allocator.hpp>
#include <seir_base/pointer.hpp>

#include <cstring>
#include <type_traits>

// TODO: All Buffers should be std::byte-typed.
// TODO: Buffers should probably have a single specialized allocator optimized for large-ish memory blocks.

namespace seir
{
	//
	template <class T, class A = Allocator>
	class Buffer
	{
	public:
		static_assert(std::is_trivially_default_constructible_v<T> && std::is_trivially_copyable_v<T> && std::is_trivially_destructible_v<T>);

		constexpr Buffer() noexcept = default;
		Buffer(const Buffer&) = delete;
		constexpr Buffer(Buffer&& other) noexcept
			: _data{ std::move(other._data) }, _capacity{ other._capacity } { other._capacity = 0; }
		Buffer& operator=(Buffer&) = delete;
		constexpr Buffer& operator=(Buffer&&) noexcept;
		~Buffer() noexcept = default;

		//
		explicit Buffer(size_t capacity)
			: _data{ static_cast<T*>(A::allocate(capacity * sizeof(T))) }, _capacity{ capacity } {}

		//
		[[nodiscard]] constexpr size_t capacity() const noexcept { return _capacity; }

		//
		[[nodiscard]] constexpr T* data() noexcept { return _data; }

		//
		[[nodiscard]] constexpr const T* data() const noexcept { return _data; }

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
		CPtr<T, A::deallocate> _data;
		size_t _capacity = 0;
	};
}

template <class T, class A>
constexpr seir::Buffer<T, A>& seir::Buffer<T, A>::operator=(Buffer&& other) noexcept
{
	swap(*this, other);
	return *this;
}

template <class T, class A>
void seir::Buffer<T, A>::reserve(size_t totalCapacity, size_t preservedCapacity)
{
	if (!tryReserve(totalCapacity, preservedCapacity)) [[unlikely]]
		throw std::bad_alloc{};
}

template <class T, class A>
bool seir::Buffer<T, A>::tryReserve(size_t totalCapacity, size_t preservedCapacity) noexcept
{
	if (totalCapacity <= _capacity)
		return true;
	decltype(_data) newData{ static_cast<T*>(A::tryAllocate(totalCapacity * sizeof(T))) };
	if (!newData) [[unlikely]]
		return false;
	if (preservedCapacity > 0)
		std::memcpy(newData, _data, (preservedCapacity < _capacity ? preservedCapacity : _capacity) * sizeof(T));
	_data = std::move(newData);
	_capacity = totalCapacity;
	return true;
}
