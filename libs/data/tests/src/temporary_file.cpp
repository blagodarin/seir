// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_data/temporary_file.hpp>

#include <seir_data/blob.hpp>
#include <seir_data/writer.hpp>

#include <array>

#include <doctest/doctest.h>

TEST_CASE("TemporaryFile")
{
	const auto file = seir::TemporaryFile::create();
	REQUIRE(file);
	const auto data = std::to_array<uint8_t>({ 1, 2, 3, 4, 5, 6, 7 });
	{
		const auto writer = seir::createFileWriter(*file);
		REQUIRE(writer);
		REQUIRE(writer->size() == 0);
		CHECK(writer->reserve(2 * data.size()));
		REQUIRE(writer->write(data.data(), data.size()));
		REQUIRE(writer->write(data.data(), data.size()));
	}
	const auto blob = seir::createFileBlob(*file);
	REQUIRE(blob);
	REQUIRE(blob->size() == 2 * data.size());
	CHECK_FALSE(std::memcmp(blob->data(), data.data(), data.size()));
	CHECK_FALSE(std::memcmp(static_cast<const std::byte*>(blob->data()) + data.size(), data.data(), data.size()));
}
