// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/allocator.hpp>
#include <seir_base/pointer.hpp>

#include <cstring>
#include <memory>
#include <type_traits>

namespace seir
{
	template <typename T, typename A = Allocator>
	class Buffer
	{
	public:
		static_assert(std::is_trivially_default_constructible_v<T> && std::is_trivially_copyable_v<T> && std::is_trivially_destructible_v<T>);

		constexpr Buffer() noexcept = default;
		~Buffer() noexcept = default;
		Buffer(const Buffer&) = delete;
		Buffer& operator=(Buffer&) = delete;

		explicit Buffer(size_t capacity)
			: _data{ static_cast<T*>(A::allocate(capacity * sizeof(T))) }, _capacity{ capacity } {}

		constexpr Buffer(Buffer&& other) noexcept
			: _data{ std::move(other._data) }, _capacity{ other._capacity } { other._capacity = 0; }

		constexpr Buffer& operator=(Buffer&& other) noexcept
		{
			swap(*this, other);
			return *this;
		}

		[[nodiscard]] constexpr size_t capacity() const noexcept { return _capacity; }
		[[nodiscard]] constexpr T* data() noexcept { return _data; }
		[[nodiscard]] constexpr const T* data() const noexcept { return _data; }

		void reserve(size_t newCapacity, bool preserveContents = true)
		{
			if (newCapacity <= _capacity)
				return;
			decltype(_data) newData{ static_cast<T*>(A::allocate(newCapacity * sizeof(T))) };
			if (preserveContents)
				std::memcpy(newData, _data, (newCapacity < _capacity ? newCapacity : _capacity) * sizeof(T));
			_data = std::move(newData);
			_capacity = newCapacity;
		}

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
