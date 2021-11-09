// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_base/pointer.hpp>

#include <cstdint>

#include <doctest/doctest.h>

namespace
{
	struct Value
	{
		unsigned _counter = 0;
	};

	template <bool kCheckNull>
	void increment(Value* pointer) noexcept
	{
		if constexpr (kCheckNull)
			CHECK(pointer); // Can't use REQUIRE in a noexcept function.
		if (pointer)
			++pointer->_counter;
	}

	struct TaggedDeleter
	{
		intptr_t _tag = 0;

		constexpr TaggedDeleter(intptr_t tag) noexcept
			: _tag{ tag } {}

		TaggedDeleter(TaggedDeleter&& deleter) noexcept
			: _tag{ std::exchange(deleter._tag, 0) } {}

		void free(Value* pointer) noexcept
		{
			::increment<false>(pointer);
		}

		friend constexpr void swap(TaggedDeleter& first, TaggedDeleter& second) noexcept
		{
			using std::swap;
			swap(first._tag, second._tag);
		}
	};
}

TEST_CASE("CPtr")
{
	using CPtr = seir::CPtr<Value, ::increment<true>>;

	const auto check = [](CPtr& ptr, Value* raw) {
		if (raw)
		{
			REQUIRE(ptr);
			CHECK(&ptr->_counter == &raw->_counter);
			CHECK(&ptr[0]._counter == &raw->_counter);
		}
		else
			CHECK_FALSE(ptr);
		CHECK(static_cast<Value*>(ptr) == raw);
		CHECK(ptr.get() == raw);
		REQUIRE(ptr.out());
		CHECK(*ptr.out() == raw);
	};

	Value value;
	Value otherValue;
	int expectedOtherValue = 0;
	{
		CPtr ptr{ &value };
		check(ptr, &value);
		SUBCASE("CPtr(CPtr&&)")
		{
			CPtr otherPtr{ std::move(ptr) };
			check(ptr, nullptr);
			check(otherPtr, &value);
			CHECK(value._counter == 0);
		}
		SUBCASE("operator=(CPtr&&)")
		{
			CPtr otherPtr;
			check(otherPtr, nullptr);
			otherPtr = std::move(ptr);
			check(ptr, nullptr);
			check(otherPtr, &value);
			CHECK(value._counter == 0);
		}
		SUBCASE("reset()")
		{
			ptr.reset();
			check(ptr, nullptr);
			CHECK(value._counter == 1);
		}
		SUBCASE("reset(T*)")
		{
			ptr.reset(&otherValue);
			check(ptr, &otherValue);
			CHECK(value._counter == 1);
			CHECK(otherValue._counter == 0);
			ptr.reset(&otherValue);
			check(ptr, &otherValue);
			CHECK(otherValue._counter == 0);
			expectedOtherValue = 1;
		}
		SUBCASE("swap()")
		{
			{
				CPtr otherPtr{ &otherValue };
				check(otherPtr, &otherValue);
				swap(ptr, otherPtr);
				check(ptr, &otherValue);
				check(otherPtr, &value);
				CHECK(value._counter == 0);
				CHECK(otherValue._counter == 0);
			}
			CHECK(value._counter == 1);
			CHECK(otherValue._counter == 0);
			expectedOtherValue = 1;
		}
	}
	CHECK(value._counter == 1);
	CHECK(otherValue._counter == expectedOtherValue);
}

TEST_CASE("TaggedPtr")
{
	using TaggedPtr = seir::Pointer<Value, TaggedDeleter>;

	const auto check = [](TaggedPtr& ptr, Value* raw, intptr_t tag) {
		if (raw)
		{
			REQUIRE(ptr);
			CHECK(&ptr->_counter == &raw->_counter);
			CHECK(&ptr[0]._counter == &raw->_counter);
		}
		else
			CHECK_FALSE(ptr);
		CHECK(static_cast<Value*>(ptr) == raw);
		CHECK(ptr.get() == raw);
		REQUIRE(ptr.out());
		CHECK(*ptr.out() == raw);
		CHECK(static_cast<const TaggedDeleter*>(static_cast<const void*>(&ptr))->_tag == tag);
	};

	Value value;
	Value otherValue;
	int expectedOtherValue = 0;
	{
		TaggedPtr ptr{ &value, 1 };
		check(ptr, &value, 1);
		SUBCASE("TaggedPtr(TaggedPtr&&)")
		{
			TaggedPtr otherPtr{ std::move(ptr) };
			check(ptr, nullptr, 0);
			check(otherPtr, &value, 1);
			CHECK(value._counter == 0);
		}
		SUBCASE("operator=(TaggedPtr&&)")
		{
			TaggedPtr otherPtr{ nullptr, 2 };
			check(otherPtr, nullptr, 2);
			otherPtr = std::move(ptr);
			check(ptr, nullptr, 2);
			check(otherPtr, &value, 1);
			CHECK(value._counter == 0);
		}
		SUBCASE("reset()")
		{
			ptr.reset();
			check(ptr, nullptr, 1);
			CHECK(value._counter == 1);
		}
		SUBCASE("reset(T*)")
		{
			ptr.reset(&otherValue);
			check(ptr, &otherValue, 1);
			CHECK(value._counter == 1);
			CHECK(otherValue._counter == 0);
			ptr.reset(&otherValue);
			check(ptr, &otherValue, 1);
			CHECK(otherValue._counter == 0);
			expectedOtherValue = 1;
		}
		SUBCASE("swap()")
		{
			{
				TaggedPtr otherPtr{ &otherValue, 2 };
				check(otherPtr, &otherValue, 2);
				swap(ptr, otherPtr);
				check(ptr, &otherValue, 2);
				check(otherPtr, &value, 1);
				CHECK(value._counter == 0);
				CHECK(otherValue._counter == 0);
			}
			CHECK(value._counter == 1);
			CHECK(otherValue._counter == 0);
			expectedOtherValue = 1;
		}
	}
	CHECK(value._counter == 1);
	CHECK(otherValue._counter == expectedOtherValue);
}
