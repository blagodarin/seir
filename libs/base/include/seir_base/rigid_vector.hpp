// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/allocator.hpp>

#include <cassert>
#include <memory>
#include <type_traits>
#include <utility>

namespace seir
{
	// std::vector-like container which:
	// * is noncopyable and is able to contain immovable objects;
	// * requires reserve() before use and allows only one reserve() during lifetime;
	// * doesn't check preconditions at runtime.
	template <typename T, typename A = Allocator>
	class RigidVector
	{
	public:
		constexpr RigidVector() noexcept = default;
		RigidVector(const RigidVector&) = delete;
		RigidVector& operator=(const RigidVector&) = delete;

		constexpr RigidVector(RigidVector&& other) noexcept
			: _data(std::exchange(other._data, nullptr))
			, _size(std::exchange(other._size, size_t{}))
#ifndef NDEBUG
			, _capacity(std::exchange(other._capacity, size_t{})) // No braces in initialization because of ClangFormat bug.
#endif
		{
		}

		~RigidVector() noexcept
		{
			std::destroy_n(_data, _size);
			A::deallocate(_data);
		}

		constexpr RigidVector& operator=(RigidVector&& other) noexcept
		{
			swap(*this, other);
			return *this;
		}

		[[nodiscard]] constexpr T* begin() noexcept { return _data; }
		[[nodiscard]] constexpr const T* begin() const noexcept { return _data; }
		[[nodiscard]] constexpr const T* cbegin() const noexcept { return _data; }
		[[nodiscard]] constexpr const T* cend() const noexcept { return _data + _size; }
		[[nodiscard]] constexpr T* data() noexcept { return _data; }
		[[nodiscard]] constexpr const T* data() const noexcept { return _data; }
		[[nodiscard]] constexpr bool empty() const noexcept { return !_size; }
		[[nodiscard]] constexpr T* end() noexcept { return _data + _size; }
		[[nodiscard]] constexpr const T* end() const noexcept { return _data + _size; }
		[[nodiscard]] constexpr size_t size() const noexcept { return _size; }

		[[nodiscard]] constexpr T& back() noexcept
		{
			assert(_size > 0);
			return _data[_size - 1];
		}

		[[nodiscard]] constexpr const T& back() const noexcept
		{
			assert(_size > 0);
			return _data[_size - 1];
		}

		void clear() noexcept
		{
			std::destroy_n(_data, _size);
			_size = 0;
		}

		template <typename... Args>
		T& emplace_back(Args&&... args)
		{
			assert(_size < _capacity);
			T* value = new (_data + _size) T{ std::forward<Args>(args)... };
			++_size;
			return *value;
		}

		void pop_back() noexcept
		{
			assert(_size > 0);
			--_size;
			std::destroy_at(_data + _size);
		}

		void reserve(size_t capacity)
		{
			assert(!_data);
			_data = static_cast<T*>(A::allocate(capacity * sizeof(T)));
#ifndef NDEBUG
			_capacity = capacity;
#endif
		}

		[[nodiscard]] T& operator[](size_t index) noexcept
		{
			assert(index < _size);
			return _data[index];
		}

		[[nodiscard]] const T& operator[](size_t index) const noexcept
		{
			assert(index < _size);
			return _data[index];
		}

		friend constexpr void swap(RigidVector& first, RigidVector& second) noexcept
		{
			using std::swap;
			swap(first._data, second._data);
			swap(first._size, second._size);
#ifndef NDEBUG
			swap(first._capacity, second._capacity);
#endif
		}

	private:
		T* _data = nullptr;
		size_t _size = 0;
#ifndef NDEBUG
		size_t _capacity = 0;
#endif
	};
}
