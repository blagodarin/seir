// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/unique_ptr.hpp>

#include <atomic>

namespace seir
{
	template <class>
	class SharedPtr;

	// Base class for SharedPtr-managed objects.
	class ReferenceCounter
	{
	public:
		ReferenceCounter(const ReferenceCounter&) = delete;
		ReferenceCounter& operator=(const ReferenceCounter&) = delete;

	protected:
		ReferenceCounter() noexcept = default;
		~ReferenceCounter() noexcept = default;

	private:
		mutable std::atomic<int> _references{ 1 };
		template <class>
		friend class SharedPtr;
		template <class To, class From>
		friend SharedPtr<To> staticCast(const SharedPtr<From>&) noexcept; // NOLINT(readability-redundant-declaration)
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
		constexpr SharedPtr(std::nullptr_t) noexcept {} // cppcheck-suppress[noExplicitConstructor]
		SharedPtr(const SharedPtr&) noexcept;
		template <class U>
		requires std::is_base_of_v<T, U>
		explicit SharedPtr(const SharedPtr<U>&) noexcept;
		constexpr SharedPtr(SharedPtr&& other) noexcept
			: _pointer{ other._pointer } { other._pointer = nullptr; }
		template <class U>
		constexpr explicit SharedPtr(SharedPtr<U>&&) noexcept;
		template <class U>
		constexpr explicit SharedPtr(UniquePtr<U>&&) noexcept;
		~SharedPtr() noexcept { reset(nullptr); }
		SharedPtr& operator=(const SharedPtr&) noexcept; // NOLINT(bugprone-unhandled-self-assignment, cert-oop54-cpp)
		template <class U>
		requires std::is_base_of_v<T, U>
		SharedPtr<T>& operator=(const SharedPtr<U>&) noexcept;
		SharedPtr& operator=(SharedPtr&&) noexcept;
		template <class U>
		requires std::is_base_of_v<T, U>
		SharedPtr<T>& operator=(SharedPtr<U>&&) noexcept;
		template <class U>
		requires std::is_base_of_v<T, U>
		SharedPtr<T>& operator=(UniquePtr<U>&&) noexcept;
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
		requires std::is_base_of_v<ReferenceCounter, R> && std::is_base_of_v<R, U>
		friend SharedPtr<R> makeShared(Args&&...); // NOLINT(readability-redundant-declaration)
		template <class To, class From>
		friend SharedPtr<To> staticCast(const SharedPtr<From>&) noexcept; // NOLINT(readability-redundant-declaration)
	};

	template <class T>
	SharedPtr(UniquePtr<T>&&) -> SharedPtr<T>;

	template <class R, class U = R, class... Args>
	requires std::is_base_of_v<ReferenceCounter, R> && std::is_base_of_v<R, U>
	[[nodiscard]] inline SharedPtr<R> makeShared(Args&&... args)
	{
		return SharedPtr<R>{ new U{ std::forward<Args>(args)... } };
	}

	template <class To, class From>
	[[nodiscard]] SharedPtr<To> staticCast(const SharedPtr<From>& from) noexcept
	{
		if (from)
			from->_references.fetch_add(1);
		return SharedPtr{ static_cast<To*>(from._pointer) };
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
template <class U>
requires std::is_base_of_v<T, U>
seir::SharedPtr<T>::SharedPtr(const SharedPtr<U>& other) noexcept
	: _pointer{ other._pointer }
{
	if (_pointer)
		_pointer->_references.fetch_add(1);
}

template <class T>
template <class U>
constexpr seir::SharedPtr<T>::SharedPtr(SharedPtr<U>&& other) noexcept // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
	: _pointer{ other._pointer }
{
	other._pointer = nullptr;
}

template <class T>
template <class U>
constexpr seir::SharedPtr<T>::SharedPtr(UniquePtr<U>&& other) noexcept // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
	: _pointer{ other._pointer }
{
	static_assert(std::is_base_of_v<T, U> && std::is_base_of_v<ReferenceCounter, T>);
	other._pointer = nullptr;
}

template <class T>
seir::SharedPtr<T>& seir::SharedPtr<T>::operator=(const SharedPtr& other) noexcept // NOLINT(bugprone-unhandled-self-assignment, cert-oop54-cpp)
{
	if (other._pointer)
		other._pointer->_references.fetch_add(1);
	reset(other._pointer);
	return *this;
}

template <class T>
template <class U>
requires std::is_base_of_v<T, U>
seir::SharedPtr<T>& seir::SharedPtr<T>::operator=(const SharedPtr<U>& other) noexcept
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
requires std::is_base_of_v<T, U>
seir::SharedPtr<T>& seir::SharedPtr<T>::operator=(SharedPtr<U>&& other) noexcept // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
{
	const auto pointer = other._pointer;
	other._pointer = nullptr;
	reset(pointer);
	return *this;
}

template <class T>
template <class U>
requires std::is_base_of_v<T, U>
seir::SharedPtr<T>& seir::SharedPtr<T>::operator=(UniquePtr<U>&& other) noexcept // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
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
