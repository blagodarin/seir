// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_data/blob.hpp>

#include <array>
#include <cstring>

#include <doctest/doctest.h>

TEST_CASE("Blob::from(const std::string&)")
{
	const auto blob = seir::Blob::from(SEIR_TEST_DIR "file.txt");
	REQUIRE(blob);
	const std::string_view expected{ "contents" };
	CHECK(blob->size() == expected.size());
	CHECK_FALSE(std::memcmp(blob->data(), expected.data(), expected.size()));
}

TEST_CASE("Blob::from(...)")
{
	SUBCASE("size() == 0")
	{
		const auto first = seir::Blob::from(nullptr, 0);
		REQUIRE(first);
		CHECK(first->data() == nullptr);
		CHECK(first->size() == 0);
		const auto second = seir::Blob::from(&first, 0);
		REQUIRE(second);
		CHECK(second->data() == &first);
		CHECK(second->size() == 0);
	}
	SUBCASE("size() > 0")
	{
		const std::array<char, 4> buffer{ 'H', 'E', 'L', 'O' };
		const auto all = seir::Blob::from(buffer.data(), buffer.size());
		REQUIRE(all);
		CHECK(all->data() == buffer.data());
		CHECK(all->size() == buffer.size());
		const auto check = [](const seir::Blob& blob, size_t offset, char expected) {
			const auto actual = blob.get<char>(offset);
			REQUIRE(actual);
			CHECK(*actual == expected);
		};
		check(*all, 0, 'H');
		check(*all, 1, 'E');
		check(*all, 2, 'L');
		check(*all, 3, 'O');
		CHECK_FALSE(all->get<char>(4));
		{
			const auto mid = seir::Blob::from(seir::SharedPtr{ all }, 1, 2);
			REQUIRE(mid);
			CHECK(mid->data() == buffer.data() + 1);
			CHECK(mid->size() == 2);
			check(*mid, 0, 'E');
			check(*mid, 1, 'L');
			CHECK_FALSE(mid->get<char>(2));
		}
		{
			const auto end = seir::Blob::from(seir::SharedPtr{ all }, 2, 4);
			REQUIRE(end);
			CHECK(end->data() == buffer.data() + 2);
			CHECK(end->size() == 2);
			check(*end, 0, 'L');
			check(*end, 1, 'O');
			CHECK_FALSE(end->get<char>(2));
		}
		{
			const auto bad = seir::Blob::from(seir::SharedPtr{ all }, 5, 4);
			REQUIRE(bad);
			CHECK(bad->data() == buffer.data() + 4);
			CHECK(bad->size() == 0);
		}
	}
}
