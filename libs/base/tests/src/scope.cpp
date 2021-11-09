// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_base/scope.hpp>

#include <doctest/doctest.h>

TEST_CASE("FINALLY")
{
	unsigned value = 0;
	SUBCASE("leave")
	{
		{
			SEIR_FINALLY([&value] { value += 1; });
			SEIR_FINALLY([&value] { value += 2; });
			CHECK(value == 0);
		}
		CHECK(value == 3);
	}
	SUBCASE("throw")
	{
		bool caught = false;
		try
		{
			SEIR_FINALLY([&value] { value += 1; });
			SEIR_FINALLY([&value] { value += 2; });
			CHECK(value == 0);
			throw 0;
		}
		catch (int)
		{
			caught = true;
			CHECK(value == 3);
		}
		CHECK(caught);
	}
}
