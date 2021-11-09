// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <utility>

namespace seir
{
	template <typename T, typename Deleter>
	class Pointer : private Deleter
	{
	public:
		constexpr Pointer() noexcept = default;
		Pointer(const Pointer&) = delete;
		~Pointer() noexcept { Deleter::free(_pointer); }
		Pointer& operator=(const Pointer&) = delete;

		template <typename... DeleterArgs>
		constexpr explicit Pointer(T* pointer, DeleterArgs&&... args) noexcept
			: Deleter{ std::forward<DeleterArgs>(args)... }, _pointer{ pointer } {}

		constexpr Pointer(Pointer&& other) noexcept
			: Deleter{ static_cast<Deleter&&>(other) }, _pointer{ std::exchange(other._pointer, nullptr) } {}

		constexpr Pointer& operator=(Pointer&& other) noexcept
		{
			swap(*this, other);
			return *this;
		}

		[[nodiscard]] constexpr operator T*() const noexcept { return _pointer; }
		[[nodiscard]] constexpr T* operator->() const noexcept { return _pointer; }
		[[nodiscard]] constexpr T* get() const noexcept { return _pointer; }
		[[nodiscard]] constexpr T** out() noexcept { return &_pointer; }

		void reset(T* pointer = nullptr) noexcept
		{
			if (_pointer != pointer)
			{
				Deleter::free(_pointer);
				_pointer = pointer;
			}
		}

		friend constexpr void swap(Pointer& first, Pointer& second) noexcept
		{
			using std::swap;
			if constexpr (sizeof(Pointer) > sizeof(void*))
				swap(static_cast<Deleter&>(first), static_cast<Deleter&>(second));
			swap(first._pointer, second._pointer);
		}

	private:
		T* _pointer = nullptr;
	};

	template <auto deleter>
	class FunctionDeleter
	{
	public:
		template <typename T>
		static void free(T* pointer) noexcept
		{
			if (pointer)
				deleter(pointer);
		}
	};

	// Smart pointer for working with C APIs.
	template <typename T, auto deleter>
	using CPtr = Pointer<T, FunctionDeleter<deleter>>;
}
