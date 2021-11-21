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
		constexpr UniquePtr(std::nullptr_t) noexcept {}
		UniquePtr(UniquePtr&& other) noexcept
			: _pointer{ other._pointer } { other._pointer = nullptr; }
		template <class U>
		UniquePtr(UniquePtr<U>&& other) noexcept
			: _pointer{ other._pointer } { other._pointer = nullptr; }
		~UniquePtr() noexcept { delete _pointer; }
		UniquePtr& operator=(UniquePtr&& other) noexcept;
		template <class U>
		UniquePtr& operator=(UniquePtr<U>&& other) noexcept;
		[[nodiscard]] constexpr T& operator*() const noexcept { return *_pointer; }
		[[nodiscard]] constexpr T* operator->() const noexcept { return _pointer; }
		[[nodiscard]] constexpr explicit operator bool() const noexcept { return _pointer; }
		[[nodiscard]] constexpr T* get() const noexcept { return _pointer; }
		void reset() noexcept;

	private:
		T* _pointer = nullptr;
		constexpr explicit UniquePtr(T* pointer) noexcept
			: _pointer{ pointer } {}
		template <class>
		friend class UniquePtr;
		template <class>
		friend class SharedPtr;
		template <class U, class... Args>
		friend UniquePtr<U> makeUnique(Args&&...);
	};

	template <class U, class... Args>
	[[nodiscard]] inline UniquePtr<U> makeUnique(Args&&... args) { return UniquePtr<U>{ new U{ std::forward<Args>(args)... } }; }
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
