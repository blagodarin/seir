// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_io/directory_view.hpp>

#include <doctest/doctest.h>

TEST_CASE("DirectoryView")
{
	std::string testDirWithoutSlash = SEIR_TEST_DIR;
	testDirWithoutSlash.pop_back();
	SUBCASE("*.txt")
	{
		seir::DirectoryView directory{ "*.txt" };
		directory.reset(SEIR_TEST_DIR);
		CHECK(directory.path() == SEIR_TEST_DIR);
		const auto& entries = directory.entries();
		REQUIRE(entries.size() == 2);
		CHECK(entries[0].name == "..");
		CHECK(entries[0].type == seir::DirectoryView::Entry::Type::Directory);
		CHECK(entries[0].fullName == testDirWithoutSlash);
		CHECK(entries[1].name == "file.txt");
		CHECK(entries[1].type == seir::DirectoryView::Entry::Type::File);
		CHECK(entries[1].fullName == SEIR_TEST_DIR "file.txt");
	}
	SUBCASE("*.xyz")
	{
		seir::DirectoryView directory{ "*.xyz" };
		directory.reset(SEIR_TEST_DIR);
		CHECK(directory.path() == SEIR_TEST_DIR);
		const auto& entries = directory.entries();
		REQUIRE(entries.size() == 1);
		CHECK(entries[0].name == "..");
		CHECK(entries[0].type == seir::DirectoryView::Entry::Type::Directory);
		CHECK(entries[0].fullName == testDirWithoutSlash);
	}
	SUBCASE("/")
	{
		seir::DirectoryView directory{ "*" };
		constexpr std::string_view rootDir =
#ifdef _WIN32
			std::string_view{ SEIR_TEST_DIR }.substr(0, 3)
#else
			"/"
#endif
			;
		directory.reset(rootDir);
		CHECK(directory.path() == rootDir);
		const auto& entries = directory.entries();
		REQUIRE_FALSE(entries.empty());
		CHECK(entries[0].name != "..");
	}
}
