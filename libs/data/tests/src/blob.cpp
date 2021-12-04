// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_data/blob.hpp>

#include <array>

#include <doctest/doctest.h>

TEST_CASE("Blob::from")
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
		const auto all = seir::SharedPtr{ seir::Blob::from(buffer.data(), buffer.size()) };
		REQUIRE(all);
		CHECK(all->data() == buffer.data());
		CHECK(all->size() == buffer.size());
		CHECK(all->get<char>(0) == 'H');
		CHECK(all->get<char>(1) == 'E');
		CHECK(all->get<char>(2) == 'L');
		CHECK(all->get<char>(3) == 'O');
		{
			const auto mid = seir::Blob::from(all, 1, 2);
			REQUIRE(mid);
			CHECK(mid->data() == buffer.data() + 1);
			CHECK(mid->size() == 2);
			CHECK(mid->get<char>(0) == 'E');
			CHECK(mid->get<char>(1) == 'L');
		}
		{
			const auto end = seir::Blob::from(all, 2, 4);
			REQUIRE(end);
			CHECK(end->data() == buffer.data() + 2);
			CHECK(end->size() == 2);
			CHECK(end->get<char>(0) == 'L');
			CHECK(end->get<char>(1) == 'O');
		}
		{
			const auto bad = seir::Blob::from(all, 5, 4);
			REQUIRE(bad);
			CHECK(bad->data() == buffer.data() + 4);
			CHECK(bad->size() == 0);
		}
	}
}