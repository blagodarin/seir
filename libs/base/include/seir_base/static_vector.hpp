// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cassert>
#include <memory>
#include <utility>

namespace seir
{
	// std::vector-like container with preallocated storage (like std::array).
	template <typename T, size_t kCapacity, size_t kAlignment = alignof(T)>
	class StaticVector
	{
	public:
		constexpr StaticVector() noexcept = default;
		~StaticVector() noexcept { clear(); }
		StaticVector& operator=(const StaticVector&) = delete;

		explicit StaticVector(const StaticVector& other) noexcept(std::is_nothrow_copy_constructible_v<T>)
			: _size{ other._size }
		{
			std::uninitialized_copy_n(other.data(), _size, reinterpret_cast<T*>(_data));
		}

		StaticVector(std::initializer_list<T> initializers) noexcept(std::is_nothrow_copy_constructible_v<T>) // cppcheck-suppress[noExplicitConstructor]
			: _size{ initializers.size() <= kCapacity ? initializers.size() : kCapacity }
		{
			std::uninitialized_copy_n(initializers.begin(), _size, reinterpret_cast<T*>(_data));
		}

		[[nodiscard]] constexpr T* begin() noexcept { return reinterpret_cast<T*>(_data); }
		[[nodiscard]] constexpr const T* begin() const noexcept { return reinterpret_cast<const T*>(_data); }
		[[nodiscard]] constexpr const T* cbegin() const noexcept { return reinterpret_cast<const T*>(_data); }
		[[nodiscard]] constexpr const T* cend() const noexcept { return reinterpret_cast<const T*>(_data) + _size; }
		[[nodiscard]] constexpr T* data() noexcept { return reinterpret_cast<T*>(_data); }
		[[nodiscard]] constexpr const T* data() const noexcept { return reinterpret_cast<const T*>(_data); }
		[[nodiscard]] constexpr bool empty() const noexcept { return !_size; }
		[[nodiscard]] constexpr T* end() noexcept { return reinterpret_cast<T*>(_data) + _size; }
		[[nodiscard]] constexpr const T* end() const noexcept { return reinterpret_cast<const T*>(_data) + _size; }
		[[nodiscard]] constexpr size_t size() const noexcept { return _size; }

		[[nodiscard]] constexpr T& back() noexcept
		{
			assert(_size > 0);
			return reinterpret_cast<T*>(_data)[_size - 1];
		}

		[[nodiscard]] constexpr const T& back() const noexcept
		{
			assert(_size > 0);
			return reinterpret_cast<const T*>(_data)[_size - 1];
		}

		constexpr void clear() noexcept
		{
			std::destroy_n(reinterpret_cast<T*>(_data), _size);
			_size = 0;
		}

		template <typename... Args>
		T& emplace_back(Args&&... args)
		{
			assert(_size < kCapacity);
			T* item = new (_data + _size * sizeof(T)) T{ std::forward<Args>(args)... }; // cppcheck-suppress[unreadVariable] // NOLINT(bugprone-sizeof-expression, cppcoreguidelines-init-variables)
			++_size;
			return *item;
		}

		void pop_back() noexcept
		{
			assert(_size > 0);
			--_size;
			std::destroy_at(reinterpret_cast<T*>(_data) + _size);
		}

		T& push_back(const T& value)
		{
			assert(_size < kCapacity);
			T* item = new (_data + _size * sizeof(T)) T{ value }; // cppcheck-suppress[unreadVariable] // NOLINT(bugprone-sizeof-expression, cppcoreguidelines-init-variables)
			++_size;
			return *item;
		}

		[[nodiscard]] constexpr T& operator[](size_t index) noexcept
		{
			assert(index < _size);
			return reinterpret_cast<T*>(_data)[index];
		}

		[[nodiscard]] constexpr const T& operator[](size_t index) const noexcept
		{
			assert(index < _size);
			return reinterpret_cast<const T*>(_data)[index];
		}

	private:
		size_t _size = 0;
		alignas(kAlignment) std::byte _data[kCapacity * sizeof(T)];
	};
}
