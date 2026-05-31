// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_io/inlet.hpp>

#include <cstring>
#include <span>

#include <ostream>
#include <doctest/doctest.h>

namespace
{
	void checkString(const std::string& name, std::string_view actual)
	{
		const auto inlet = seir::fromFile(SEIR_TEST_DIR + name);
		REQUIRE(inlet);
		const std::string_view expected{ static_cast<const char*>(inlet->data()), inlet->size() };
		CHECK(expected == actual);
	}

	template <size_t N>
	void checkUint8(const std::string& name, std::span<const uint8_t, N> actual)
	{
		const auto inlet = seir::fromFile(SEIR_TEST_DIR + name);
		REQUIRE(inlet);
		REQUIRE(inlet->size() == actual.size_bytes());
		CHECK(std::memcmp(inlet->data(), actual.data(), inlet->size()) == 0);
	}
}

TEST_CASE("string")
{
	// TODO: Use null-terminated string view if it becomes available.
	constexpr std::span embedded{
#include "string.txt.inc"
	};
	constexpr std::string_view string{ embedded.data(), embedded.size() - 1 }; // Cut null-terminator.
	::checkString("string.txt", string);
}

TEST_CASE("uint8")
{
	constexpr uint8_t embedded[]{
#include "uint8.txt.inc"
	};
	::checkUint8("uint8.txt", std::span{ embedded });
}
