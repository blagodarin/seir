// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_data/blob.hpp>
#include <seir_data/file.hpp>
#include <seir_data/writer.hpp>

#include <array>

#include <doctest/doctest.h>

TEST_CASE("TemporaryFile")
{
	std::filesystem::path path;
	{
		const auto file = seir::TemporaryFile::create();
		REQUIRE(file);
		path = file->path();
		MESSAGE("TemporaryFile: ", path);
		REQUIRE(std::filesystem::exists(path));
		CHECK(std::filesystem::file_size(path) == 0);
		const auto data = std::to_array<uint8_t>({ 1, 2, 3, 4, 5, 6, 7 });
		{
			const auto writer = seir::createFileWriter(*file);
			REQUIRE(writer);
			CHECK(writer->reserve(2 * data.size()));
			REQUIRE(writer->write(data.data(), data.size()));
			REQUIRE(writer->write(data.data(), data.size()));
		}
		REQUIRE(std::filesystem::exists(path));
		CHECK(std::filesystem::file_size(path) == 2 * data.size());
		const auto blob = seir::createFileBlob(*file);
		REQUIRE(blob);
		REQUIRE(blob->size() == 2 * data.size());
		CHECK_FALSE(std::memcmp(blob->data(), data.data(), data.size()));
		CHECK_FALSE(std::memcmp(static_cast<const std::byte*>(blob->data()) + data.size(), data.data(), data.size()));
	}
	CHECK_FALSE(std::filesystem::exists(path));
}

TEST_CASE("createFileBlob(const std::filesystem::path&)")
{
	const auto blob = seir::createFileBlob(SEIR_TEST_DIR "file.txt");
	REQUIRE(blob);
	const std::string_view expected{ "contents" };
	CHECK(blob->size() == expected.size());
	CHECK_FALSE(std::memcmp(blob->data(), expected.data(), expected.size()));
}

TEST_CASE("createFileWriter(const std::filesystem::path&)")
{
	const auto path = std::filesystem::temp_directory_path() / "test.txt";
	const std::string_view data{ "Hello world!\n" };
	{
		const auto writer = seir::createFileWriter(path);
		REQUIRE(writer);
		CHECK(writer->reserve(2 * data.size()));
		REQUIRE(writer->write(data.data(), data.size()));
		REQUIRE(writer->write(data.data(), data.size()));
	}
	REQUIRE(std::filesystem::exists(path));
	CHECK(std::filesystem::file_size(path) == 2 * data.size());
	const auto blob = seir::createFileBlob(path);
	REQUIRE(blob);
	REQUIRE(blob->size() == 2 * data.size());
	CHECK_FALSE(std::memcmp(blob->data(), data.data(), data.size()));
	CHECK_FALSE(std::memcmp(static_cast<const std::byte*>(blob->data()) + data.size(), data.data(), data.size()));
}
