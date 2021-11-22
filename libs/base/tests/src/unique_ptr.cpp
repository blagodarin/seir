// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_base/unique_ptr.hpp>

#include <doctest/doctest.h>

namespace
{
	class Base
	{
	public:
		constexpr explicit Base(int& counter) noexcept
			: _counter{ counter } { ++_counter; }
		virtual ~Base() noexcept { --_counter; }

	protected:
		int& _counter;
	};

	class Derived : public Base
	{
	public:
		constexpr explicit Derived(int& counter) noexcept
			: Base{ counter } { ++_counter; }
		~Derived() noexcept override { --_counter; }
	};
}

TEST_CASE("UniquePtr")
{
	SUBCASE("UniquePtr()")
	{
		seir::UniquePtr<Base> ptr;
		CHECK_FALSE(ptr);
		CHECK_FALSE(ptr.get());
		CHECK_FALSE(&ptr.operator*());
		CHECK_FALSE(ptr.operator->());
		ptr = std::move(ptr);
		CHECK_FALSE(ptr);
		decltype(ptr) other{ std::move(ptr) };
		CHECK_FALSE(ptr);
		CHECK_FALSE(other);
		ptr = std::move(other);
		CHECK_FALSE(ptr);
		CHECK_FALSE(other);
		ptr.reset();
		CHECK_FALSE(ptr);
	}
	SUBCASE("UniquePtr(nullptr)")
	{
		const seir::UniquePtr<Base> ptr{ nullptr };
		CHECK_FALSE(ptr);
	}
	SUBCASE("makeUnique")
	{
		int counter = 0;
		SUBCASE("<Base>")
		{
			const auto ptr = seir::makeUnique<Base>(counter);
			static_assert(std::is_same_v<decltype(*ptr), Base&>);
			CHECK(counter == 1);
		}
		SUBCASE("<Base, Base>")
		{
			const auto ptr = seir::makeUnique<Base, Base>(counter);
			static_assert(std::is_same_v<decltype(*ptr), Base&>);
			CHECK(counter == 1);
		}
		SUBCASE("<Base, Derived>")
		{
			const auto ptr = seir::makeUnique<Base, Derived>(counter);
			static_assert(std::is_same_v<decltype(*ptr), Base&>);
			CHECK(counter == 2);
		}
		SUBCASE("<Derived>")
		{
			const auto ptr = seir::makeUnique<Derived>(counter);
			static_assert(std::is_same_v<decltype(*ptr), Derived&>);
			CHECK(counter == 2);
		}
		CHECK(counter == 0);
	}
	SUBCASE("...")
	{
		int counter = 0;
		{
			auto ptr = seir::makeUnique<Derived>(counter);
			CHECK(ptr);
			CHECK(ptr.get());
			CHECK(ptr.get() == &ptr.operator*());
			CHECK(ptr.get() == ptr.operator->());
			CHECK(counter == 2);
			SUBCASE("construction")
			{
				const decltype(ptr) other{ std::move(ptr) };
				CHECK_FALSE(ptr);
				CHECK(other);
				CHECK(counter == 2);
			}
			SUBCASE("construction-cast")
			{
				const seir::UniquePtr<Base> base{ std::move(ptr) };
				CHECK_FALSE(ptr);
				CHECK(base);
				CHECK(counter == 2);
			}
			SUBCASE("assignment")
			{
				int otherCounter = 0;
				auto other = seir::makeUnique<Derived>(otherCounter);
				CHECK(other);
				CHECK(otherCounter == 2);
				other = std::move(ptr);
				CHECK_FALSE(ptr);
				CHECK(other);
				CHECK(otherCounter == 0);
				CHECK(counter == 2);
			}
			SUBCASE("assignment-cast")
			{
				int baseCounter = 0;
				auto base = seir::makeUnique<Base>(baseCounter);
				CHECK(base);
				CHECK(baseCounter == 1);
				base = std::move(ptr);
				CHECK_FALSE(ptr);
				CHECK(base);
				CHECK(baseCounter == 0);
				CHECK(counter == 2);
			}
			SUBCASE("assignment-null")
			{
				decltype(ptr) other;
				CHECK_FALSE(other);
				SUBCASE("from")
				{
					ptr = std::move(other);
					CHECK_FALSE(other);
					CHECK_FALSE(ptr);
				}
				SUBCASE("into")
				{
					other = std::move(ptr);
					CHECK_FALSE(ptr);
					CHECK(other);
					CHECK(counter == 2);
				}
			}
			SUBCASE("assignment-self")
			{
				ptr = std::move(ptr);
				CHECK(ptr);
				CHECK(counter == 2);
			}
			SUBCASE("reset")
			{
				ptr.reset();
				CHECK_FALSE(ptr);
			}
		}
		CHECK(counter == 0);
	}
}
