// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_data/blob.hpp>
#include <seir_base/endian.hpp>
#include <seir_data/file.hpp>
#include <seir_data/writer.hpp>
#include "common.hpp"

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
	const auto self = seir::createFileBlob(thisExecutable);
	REQUIRE(self);
	CHECK(self->size() == std::filesystem::file_size(thisExecutable));
#ifdef _WIN32
	REQUIRE(self->size() >= 0x40); // MS-DOS stub header.
	CHECK(self->get<uint16_t>(0) == seir::makeCC('M', 'Z'));
	const auto peSignatureOffset = self->get<uint32_t>(0x3c);
	REQUIRE(self->size() > peSignatureOffset + 4);
	CHECK(self->get<uint32_t>(peSignatureOffset) == seir::makeCC('P', 'E', '\0', '\0'));
#else
	REQUIRE(self->size() >= 4);
	CHECK(self->get<uint32_t>(0) == seir::makeCC('\x7f', 'E', 'L', 'F'));
#endif
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
