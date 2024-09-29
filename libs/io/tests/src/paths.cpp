// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_io/paths.hpp>

#include <seir_io/writer.hpp>

#include <filesystem>

#include <doctest/doctest.h>

namespace
{
	auto toPath(const std::string& path)
	{
		return std::filesystem::path{ reinterpret_cast<const char8_t*>(path.c_str()) };
	};

	void testWritablePath(std::optional<std::string> (*pathConstructor)(std::string_view))
	{
		const auto base = pathConstructor("/");
		REQUIRE(base.has_value());
		const auto fsBase = ::toPath(*base);
		REQUIRE(std::filesystem::exists(fsBase));
		REQUIRE_NOTHROW(std::filesystem::remove_all(fsBase / "TestVendor"));
		const auto path = pathConstructor("TestVendor/TestApp/file.txt");
		REQUIRE(path.has_value());
		MESSAGE(*path);
		const auto fsPath = ::toPath(*path);
		REQUIRE(std::filesystem::exists(fsPath.parent_path()));
		REQUIRE(!std::filesystem::exists(fsPath));
		const auto writer = seir::Writer::create(*path);
		CHECK(writer);
		CHECK(std::filesystem::exists(fsPath));
	}
}

TEST_CASE("makeScreenshotPath")
{
	::testWritablePath(seir::makeScreenshotPath);
}

TEST_CASE("makeUserDataPath")
{
	::testWritablePath(seir::makeUserDataPath);
}

TEST_CASE("makeUserStatePath")
{
	::testWritablePath(seir::makeUserStatePath);
}
