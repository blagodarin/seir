// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_base/static_vector.hpp>

#include <vector>

#include <doctest/doctest.h>

namespace
{
	using StaticVector = seir::StaticVector<int, 2>;

	void check(StaticVector& vector, const std::vector<int>& expected)
	{
		CHECK(vector.empty() == expected.empty());
		CHECK(vector.size() == expected.size());
		auto it = vector.begin();
		auto cit = vector.cbegin();
		auto data = vector.data();
		CHECK(data);
		CHECK(std::as_const(vector).begin() == cit);
		CHECK(std::as_const(vector).data() == data);
		for (size_t index = 0; index < expected.size(); ++index)
		{
			INFO('[', index, ']');
			REQUIRE(it < vector.end());
			REQUIRE(cit < vector.cend());
			REQUIRE(data);
			const auto value = expected[index];
			CHECK(*it == value);
			CHECK(*cit == value);
			CHECK(*data == value);
			CHECK(vector[index] == value);
			CHECK(std::as_const(vector)[index] == value);
			++it;
			++cit;
			++data;
		}
		CHECK(vector.end() == it);
		CHECK(vector.cend() == cit);
		CHECK(std::as_const(vector).end() == cit);
		if (!expected.empty())
		{
			const auto value = expected.back();
			CHECK(vector.back() == value);
			CHECK(std::as_const(vector).back() == value);
		}
	}
}

TEST_CASE("StaticVector")
{
	StaticVector vector;
	::check(vector, {});
	SUBCASE("clear()")
	{
		vector.clear();
		::check(vector, {});
	}
	SUBCASE("emplace_back()")
	{
		vector.emplace_back(1);
		::check(vector, { 1 });
		vector.emplace_back(2);
		::check(vector, { 1, 2 });
		SUBCASE("clear()")
		{
			vector.clear();
			::check(vector, {});
		}
		SUBCASE("pop_back()")
		{
			vector.pop_back();
			::check(vector, { 1 });
			vector.pop_back();
			::check(vector, {});
		}
	}
}
