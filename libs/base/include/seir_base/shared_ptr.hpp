// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/unique_ptr.hpp>

#include <atomic>

namespace seir
{
	// Base class for SharedPtr-managed objects.
	class ReferenceCounter
	{
	public:
		ReferenceCounter(const ReferenceCounter&) = delete;
		ReferenceCounter& operator=(const ReferenceCounter&) = delete;

	protected:
		ReferenceCounter() noexcept = default;
		virtual ~ReferenceCounter() noexcept = default;

	private:
		mutable std::atomic<int> _references{ 1 };
		template <class>
		friend class SharedPtr;
	};

	// Smart pointer for ReferenceCounter-derived classes that provides the following advantages over std::shared_ptr:
	// - enables unique-shared semantics (creating and using as unique until needed to share)
	//   without requiring an extra allocation on conversion;
	// - sizeof(SharedPtr) == sizeof(void*).
	template <class T>
	class SharedPtr
	{
	public:
		constexpr SharedPtr() noexcept = default;
		// cppcheck-suppress noExplicitConstructor
		constexpr SharedPtr(std::nullptr_t) noexcept {}
		SharedPtr(const SharedPtr& other) noexcept;
		template <class U, class = std::enable_if_t<std::is_base_of_v<T, U>>>
		explicit SharedPtr(const SharedPtr<U>&) noexcept;
		constexpr SharedPtr(SharedPtr&& other) noexcept
			: _pointer{ other._pointer } { other._pointer = nullptr; }
		template <class U>
		constexpr explicit SharedPtr(SharedPtr<U>&& other) noexcept
			: _pointer{ other._pointer } { other._pointer = nullptr; }
		template <class U>
		constexpr explicit SharedPtr(UniquePtr<U>&&) noexcept;
		~SharedPtr() noexcept { reset(nullptr); }
		SharedPtr& operator=(const SharedPtr& other) noexcept;
		template <class U>
		std::enable_if_t<std::is_base_of_v<T, U>, SharedPtr<T>>& operator=(const SharedPtr<U>& other) noexcept;
		SharedPtr& operator=(SharedPtr&& other) noexcept;
		template <class U>
		std::enable_if_t<std::is_base_of_v<T, U>, SharedPtr<T>>& operator=(SharedPtr<U>&& other) noexcept;
		template <class U>
		std::enable_if_t<std::is_base_of_v<T, U>, SharedPtr<T>>& operator=(UniquePtr<U>&& other) noexcept;
		[[nodiscard]] constexpr T& operator*() const noexcept { return *_pointer; }
		[[nodiscard]] constexpr T* operator->() const noexcept { return _pointer; }
		[[nodiscard]] constexpr explicit operator bool() const noexcept { return static_cast<bool>(_pointer); }
		[[nodiscard]] constexpr T* get() const noexcept { return _pointer; }
		void reset() noexcept { reset(nullptr); }
		constexpr void swap(SharedPtr& other) noexcept { std::swap(_pointer, other._pointer); }

	private:
		T* _pointer = nullptr;
		constexpr explicit SharedPtr(T* pointer) noexcept
			: _pointer{ pointer } {}
		void reset(T* pointer) noexcept;
		template <class>
		friend class SharedPtr;
		template <class R, class U, class... Args>
		friend std::enable_if_t<std::is_base_of_v<ReferenceCounter, R> && std::is_base_of_v<R, U>, SharedPtr<R>> makeShared(Args&&...);
	};

	template <class T>
	SharedPtr(UniquePtr<T>&&) -> SharedPtr<T>;

	template <class R, class U = R, class... Args>
	[[nodiscard]] inline std::enable_if_t<std::is_base_of_v<ReferenceCounter, R> && std::is_base_of_v<R, U>, SharedPtr<R>> makeShared(Args&&... args)
	{
		return SharedPtr<R>{ new U{ std::forward<Args>(args)... } };
	}

	template <class A, class B>
	[[nodiscard]] constexpr bool operator==(const SharedPtr<A>& a, const SharedPtr<B>& b) noexcept
	{
		return a.get() == b.get();
	}
}

template <class T>
seir::SharedPtr<T>::SharedPtr(const SharedPtr& other) noexcept
	: _pointer{ other._pointer }
{
	if (_pointer)
		_pointer->_references.fetch_add(1);
}

template <class T>
template <class U, class>
seir::SharedPtr<T>::SharedPtr(const SharedPtr<U>& other) noexcept
	: _pointer{ other._pointer }
{
	if (_pointer)
		_pointer->_references.fetch_add(1);
}

template <class T>
template <class U>
constexpr seir::SharedPtr<T>::SharedPtr(UniquePtr<U>&& other) noexcept
	: _pointer{ other._pointer }
{
	static_assert(std::is_base_of_v<T, U> && std::is_base_of_v<ReferenceCounter, T>);
	other._pointer = nullptr;
}

template <class T>
seir::SharedPtr<T>& seir::SharedPtr<T>::operator=(const SharedPtr& other) noexcept
{
	if (other._pointer)
		other._pointer->_references.fetch_add(1);
	reset(other._pointer);
	return *this;
}

template <class T>
template <class U>
std::enable_if_t<std::is_base_of_v<T, U>, seir::SharedPtr<T>>& seir::SharedPtr<T>::operator=(const SharedPtr<U>& other) noexcept
{
	if (other._pointer)
		other._pointer->_references.fetch_add(1);
	reset(other._pointer);
	return *this;
}

template <class T>
seir::SharedPtr<T>& seir::SharedPtr<T>::operator=(SharedPtr&& other) noexcept
{
	const auto pointer = other._pointer;
	other._pointer = nullptr;
	reset(pointer);
	return *this;
}

template <class T>
template <class U>
std::enable_if_t<std::is_base_of_v<T, U>, seir::SharedPtr<T>>& seir::SharedPtr<T>::operator=(SharedPtr<U>&& other) noexcept
{
	const auto pointer = other._pointer;
	other._pointer = nullptr;
	reset(pointer);
	return *this;
}

template <class T>
template <class U>
std::enable_if_t<std::is_base_of_v<T, U>, seir::SharedPtr<T>>& seir::SharedPtr<T>::operator=(UniquePtr<U>&& other) noexcept
{
	const auto pointer = other._pointer;
	other._pointer = nullptr;
	reset(pointer);
	return *this;
}

template <class T>
void seir::SharedPtr<T>::reset(T* pointer) noexcept
{
	if (_pointer && _pointer->_references.fetch_sub(1) == 1)
		delete _pointer;
	_pointer = pointer;
}
