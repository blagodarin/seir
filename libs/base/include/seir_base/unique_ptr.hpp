// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <type_traits>
#include <utility>

namespace seir
{
	// Lightweight alternative to std::unique_ptr which complements SharedPtr.
	template <class T>
	class UniquePtr
	{
	public:
		constexpr UniquePtr() noexcept = default;
		constexpr UniquePtr(std::nullptr_t) noexcept {} // cppcheck-suppress[noExplicitConstructor]
		constexpr UniquePtr(UniquePtr&& other) noexcept
			: _pointer{ other._pointer } { other._pointer = nullptr; }
		template <class U>
		constexpr explicit UniquePtr(UniquePtr<U>&& other) noexcept
			: _pointer{ other._pointer } { other._pointer = nullptr; }
		~UniquePtr() noexcept { delete _pointer; }
		UniquePtr& operator=(UniquePtr&& other) noexcept;
		template <class U>
		UniquePtr& operator=(UniquePtr<U>&& other) noexcept;
		[[nodiscard]] constexpr T& operator*() const noexcept { return *_pointer; }
		[[nodiscard]] constexpr T* operator->() const noexcept { return _pointer; }
		[[nodiscard]] constexpr explicit operator bool() const noexcept { return static_cast<bool>(_pointer); }
		template <typename U = T>
		[[nodiscard]] constexpr U* get() const noexcept { return static_cast<U*>(_pointer); }
		void reset() noexcept;
		constexpr void swap(UniquePtr& other) noexcept { std::swap(_pointer, other._pointer); }

	private:
		T* _pointer = nullptr;
		constexpr explicit UniquePtr(T* pointer) noexcept
			: _pointer{ pointer } {}
		template <class>
		friend class UniquePtr;
		template <class>
		friend class SharedPtr;
		template <class R, class U, class... Args>
		friend std::enable_if_t<std::is_base_of_v<R, U>, UniquePtr<R>> makeUnique(Args&&...); // NOLINT(readability-redundant-declaration)
		template <class To, class From>
		friend UniquePtr<To> staticCast(UniquePtr<From>&&) noexcept; // NOLINT(readability-redundant-declaration)
	};

	template <class R, class U = R, class... Args>
	[[nodiscard]] std::enable_if_t<std::is_base_of_v<R, U>, UniquePtr<R>> makeUnique(Args&&... args)
	{
		return UniquePtr<R>{ new U{ std::forward<Args>(args)... } };
	}

	template <class To, class From>
	UniquePtr<To> staticCast(UniquePtr<From>&& from) noexcept
	{
		return UniquePtr{ static_cast<To*>(std::exchange(from._pointer, nullptr)) };
	}
}

template <class T>
seir::UniquePtr<T>& seir::UniquePtr<T>::operator=(UniquePtr&& other) noexcept
{
	const auto pointer = other._pointer;
	other._pointer = nullptr;
	delete _pointer;
	_pointer = pointer;
	return *this;
}

template <class T>
template <class U>
seir::UniquePtr<T>& seir::UniquePtr<T>::operator=(UniquePtr<U>&& other) noexcept
{
	const auto pointer = other._pointer;
	other._pointer = nullptr;
	delete _pointer;
	_pointer = pointer;
	return *this;
}

template <class T>
void seir::UniquePtr<T>::reset() noexcept
{
	delete _pointer;
	_pointer = nullptr;
}
