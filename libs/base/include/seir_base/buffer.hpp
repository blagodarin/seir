// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstddef>
#include <new>

namespace seir
{
	//
	class Buffer
	{
	public:
		constexpr Buffer() noexcept = default;
		Buffer(const Buffer&) = delete;
		constexpr Buffer(Buffer&& other) noexcept
			: _data{ other._data }, _capacity{ other._capacity } { other._data = nullptr; }
		Buffer& operator=(Buffer&) = delete;
		constexpr Buffer& operator=(Buffer&&) noexcept;
		~Buffer() noexcept;

		//
		explicit Buffer(size_t capacity);

		//
		[[nodiscard]] constexpr size_t capacity() const noexcept { return _capacity; }

		//
		[[nodiscard]] constexpr std::byte* data() noexcept { return _data; }

		//
		[[nodiscard]] constexpr const std::byte* data() const noexcept { return _data; }

		//
		inline void reserve(size_t totalCapacity, size_t preservedCapacity);

		//
		[[nodiscard]] bool tryReserve(size_t totalCapacity, size_t preservedCapacity) noexcept;

		friend constexpr void swap(Buffer& first, Buffer& second) noexcept
		{
			const auto firstData = first._data;
			const auto firstCapacity = first._capacity;
			first._data = second._data;
			first._capacity = second._capacity;
			second._data = firstData;
			second._capacity = firstCapacity;
		}

	private:
		std::byte* _data = nullptr;
		size_t _capacity = 0;
	};
}

constexpr seir::Buffer& seir::Buffer::operator=(Buffer&& other) noexcept
{
	_data = other._data;
	_capacity = other._capacity;
	other._data = nullptr;
	return *this;
}

void seir::Buffer::reserve(size_t totalCapacity, size_t preservedCapacity)
{
	if (!tryReserve(totalCapacity, preservedCapacity)) [[unlikely]]
		throw std::bad_alloc{};
}
