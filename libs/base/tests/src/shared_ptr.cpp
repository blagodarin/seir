// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_base/shared_ptr.hpp>

#include <doctest/doctest.h>

namespace
{
	class Base : public seir::ReferenceCounter
	{
	public:
		constexpr explicit Base(int& counter) noexcept
			: _counter{ counter } { ++_counter; }
		~Base() noexcept override { --_counter; }

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

	class CheckIncomplete
	{
		seir::SharedPtr<class Incomplete> _incomplete;
	};
}

TEST_CASE("SharedPtr")
{
	SUBCASE("SharedPtr()")
	{
		seir::SharedPtr<Base> ptr;
		CHECK_FALSE(ptr);
		CHECK_FALSE(ptr.get());
		CHECK_FALSE(ptr.operator->());
		SUBCASE("self-copy")
		{
			// cppcheck-suppress selfAssignment
			ptr = ptr;
		}
		SUBCASE("self-move")
		{
			ptr = std::move(ptr);
		}
		CHECK_FALSE(ptr);
		SUBCASE("copy")
		{
			decltype(ptr) other{ ptr };
			CHECK_FALSE(ptr);
			CHECK_FALSE(other);
			ptr = other;
			CHECK_FALSE(ptr);
			CHECK_FALSE(other);
		}
		SUBCASE("move")
		{
			CHECK_FALSE(ptr);
			decltype(ptr) other{ std::move(ptr) };
			CHECK_FALSE(ptr);
			CHECK_FALSE(other);
			ptr = std::move(other);
			CHECK_FALSE(ptr);
			CHECK_FALSE(other);
		}
	}
	SUBCASE("SharedPtr(nullptr)")
	{
		const seir::SharedPtr<Base> ptr{ nullptr };
		CHECK_FALSE(ptr);
	}
	SUBCASE("makeShared")
	{
		int counter = 0;
		SUBCASE("<Base>")
		{
			const auto ptr = seir::makeShared<Base>(counter);
			static_assert(std::is_same_v<decltype(*ptr), Base&>);
			CHECK(counter == 1);
		}
		SUBCASE("<Base, Base>")
		{
			const auto ptr = seir::makeShared<Base, Base>(counter);
			static_assert(std::is_same_v<decltype(*ptr), Base&>);
			CHECK(counter == 1);
		}
		SUBCASE("<Base, Derived>")
		{
			const auto ptr = seir::makeShared<Base, Derived>(counter);
			static_assert(std::is_same_v<decltype(*ptr), Base&>);
			CHECK(counter == 2);
		}
		SUBCASE("<Derived>")
		{
			const auto ptr = seir::makeShared<Derived>(counter);
			static_assert(std::is_same_v<decltype(*ptr), Derived&>);
			CHECK(counter == 2);
		}
		CHECK(counter == 0);
	}
	SUBCASE("...")
	{
		int counter = 0;
		{
			auto ptr = seir::makeShared<Derived>(counter);
			CHECK(ptr);
			CHECK(ptr.get());
			CHECK(ptr.get() == &ptr.operator*());
			CHECK(ptr.get() == ptr.operator->());
			CHECK(counter == 2);
			SUBCASE("construction-copy")
			{
				const decltype(ptr) other{ ptr };
				CHECK(ptr);
				CHECK(other);
				CHECK(counter == 2);
			}
			SUBCASE("construction-move")
			{
				const decltype(ptr) other{ std::move(ptr) };
				CHECK_FALSE(ptr);
				CHECK(other);
				CHECK(counter == 2);
			}
			SUBCASE("construction-cast-copy")
			{
				const seir::SharedPtr<Base> base{ ptr };
				CHECK(ptr);
				CHECK(base);
				CHECK(counter == 2);
			}
			SUBCASE("construction-cast-move")
			{
				const seir::SharedPtr<Base> base{ std::move(ptr) };
				CHECK_FALSE(ptr);
				CHECK(base);
				CHECK(counter == 2);
			}
			SUBCASE("assignment")
			{
				int otherCounter = 0;
				auto other = seir::makeShared<Derived>(otherCounter);
				CHECK(other);
				CHECK(otherCounter == 2);
				SUBCASE("copy")
				{
					other = ptr;
					CHECK(ptr);
				}
				SUBCASE("move")
				{
					other = std::move(ptr);
					CHECK_FALSE(ptr);
				}
				CHECK(other);
				CHECK(otherCounter == 0);
				CHECK(counter == 2);
			}
			SUBCASE("assignment-cast")
			{
				int baseCounter = 0;
				auto base = seir::makeShared<Base>(baseCounter);
				CHECK(base);
				CHECK(baseCounter == 1);
				SUBCASE("copy")
				{
					base = ptr;
					CHECK(ptr);
				}
				SUBCASE("move")
				{
					base = std::move(ptr);
					CHECK_FALSE(ptr);
				}
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
					SUBCASE("copy") { ptr = other; }
					SUBCASE("move") { ptr = std::move(other); }
					CHECK_FALSE(other);
					CHECK_FALSE(ptr);
				}
				SUBCASE("into")
				{
					SUBCASE("copy")
					{
						other = ptr;
						CHECK(ptr);
					}
					SUBCASE("move")
					{
						other = std::move(ptr);
						CHECK_FALSE(ptr);
					}
					CHECK(other);
					CHECK(counter == 2);
				}
			}
			SUBCASE("assignment-self")
			{
				SUBCASE("copy")
				{
					// cppcheck-suppress selfAssignment
					ptr = ptr;
				}
				SUBCASE("move")
				{
					ptr = std::move(ptr);
				}
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
	SUBCASE("SharedPtr(UniquePtr)")
	{
		int uniqueCounter = 0;
		auto uniquePtr = seir::makeUnique<Derived>(uniqueCounter);
		CHECK(uniqueCounter == 2);
		SUBCASE("construction")
		{
			const seir::SharedPtr<Base> sharedPtr{ std::move(uniquePtr) };
			CHECK_FALSE(uniquePtr);
			CHECK(sharedPtr);
			CHECK(uniqueCounter == 2);
		}
		SUBCASE("assignment")
		{
			int sharedCounter = 0;
			auto sharedPtr = seir::makeShared<Base>(sharedCounter);
			CHECK(sharedPtr);
			CHECK(sharedCounter == 1);
			sharedPtr = std::move(uniquePtr);
			CHECK_FALSE(uniquePtr);
			CHECK(sharedPtr);
			CHECK(sharedCounter == 0);
			CHECK(uniqueCounter == 2);
		}
		SUBCASE("assignment-null")
		{
			seir::SharedPtr<Base> sharedPtr;
			CHECK_FALSE(sharedPtr);
			sharedPtr = std::move(uniquePtr);
			CHECK(sharedPtr);
			CHECK(uniqueCounter == 2);
		}
		CHECK(uniqueCounter == 0);
	}
	SUBCASE("SharedPtr(UniquePtr(nullptr))")
	{
		seir::UniquePtr<Base> uniquePtr;
		CHECK_FALSE(uniquePtr);
		const seir::SharedPtr<Base> sharedPtr{ std::move(uniquePtr) };
		CHECK_FALSE(sharedPtr);
	}
}
