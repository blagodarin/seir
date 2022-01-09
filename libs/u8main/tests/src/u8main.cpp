// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_u8main/u8main.hpp>

#include <string>
#include <vector>

#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

namespace
{
	std::vector<std::string> args;
}

int u8main(int argc, char** argv)
{
	for (int i = 1; i < argc; ++i)
		args.emplace_back(argv[i]);
	doctest::Context context;
	context.applyCommandLine(argc, argv);
	return context.run();
}

TEST_CASE("u8main")
{
	REQUIRE(args.size() == 3);
	CHECK(args[0] == "English");
	CHECK(args[1] == "Русский");
	CHECK(args[2] == "日本語");
}
