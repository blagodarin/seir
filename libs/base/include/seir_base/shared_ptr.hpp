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
		constexpr SharedPtr(std::nullptr_t) noexcept {}
		SharedPtr(const SharedPtr& other) noexcept;
		template <class U>
		SharedPtr(const SharedPtr<U>&) noexcept;
		constexpr SharedPtr(SharedPtr&& other) noexcept
			: _pointer{ other._pointer } { other._pointer = nullptr; }
		template <class U>
		constexpr SharedPtr(SharedPtr<U>&& other) noexcept
			: _pointer{ other._pointer } { other._pointer = nullptr; }
		template <class U, class = std::enable_if_t<std::is_convertible_v<U*, T*> && std::is_convertible_v<T*, ReferenceCounter*>>>
		constexpr SharedPtr(UniquePtr<U>&& other) noexcept
			: _pointer{ other._pointer } { other._pointer = nullptr; }
		~SharedPtr() noexcept { reset(nullptr); }
		SharedPtr& operator=(const SharedPtr& other) noexcept;
		template <class U>
		SharedPtr& operator=(const SharedPtr<U>& other) noexcept;
		SharedPtr& operator=(SharedPtr&& other) noexcept;
		template <class U>
		SharedPtr& operator=(SharedPtr<U>&& other) noexcept;
		[[nodiscard]] constexpr T& operator*() const noexcept { return *_pointer; }
		[[nodiscard]] constexpr T* operator->() const noexcept { return _pointer; }
		[[nodiscard]] constexpr explicit operator bool() const noexcept { return _pointer; }
		[[nodiscard]] constexpr T* get() const noexcept { return _pointer; }
		void reset() noexcept { reset(nullptr); }

	private:
		T* _pointer = nullptr;
		constexpr explicit SharedPtr(T* pointer) noexcept
			: _pointer{ pointer } {}
		void reset(T* pointer) noexcept;
		template <class>
		friend class SharedPtr;
		template <class U, class... Args>
		friend std::enable_if_t<std::is_convertible_v<U*, ReferenceCounter*>, SharedPtr<U>> makeShared(Args&&...);
	};

	template <class T>
	SharedPtr(UniquePtr<T>&&) -> SharedPtr<T>;

	template <class U, class... Args>
	[[nodiscard]] inline std::enable_if_t<std::is_convertible_v<U*, ReferenceCounter*>, SharedPtr<U>> makeShared(Args&&... args) { return SharedPtr{ new U{ std::forward<Args>(args)... } }; }
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
seir::SharedPtr<T>::SharedPtr(const SharedPtr<U>& other) noexcept
	: _pointer{ other._pointer }
{
	if (_pointer)
		_pointer->_references.fetch_add(1);
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
seir::SharedPtr<T>& seir::SharedPtr<T>::operator=(SharedPtr<U>&& other) noexcept
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
