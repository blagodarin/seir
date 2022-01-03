// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_data/blob.hpp>
#include <seir_data/temporary.hpp>

#include <array>
#include <filesystem>

#include <doctest/doctest.h>

TEST_CASE("TemporaryFile")
{
	auto writer = seir::TemporaryWriter::create();
	REQUIRE(writer);
	REQUIRE(writer->size() == 0);
	const auto data = std::to_array<uint8_t>({ 1, 2, 3, 4, 5, 6, 7 });
	CHECK(writer->reserve(2 * data.size()));
	REQUIRE(writer->write(data.data(), data.size()));
	REQUIRE(writer->write(data.data(), data.size()));
	CHECK(writer->flush()); // Should successfully do nothing. Unfortunately we're unable to check that it actually does nothing.
	auto file = seir::TemporaryWriter::commit(std::move(writer));
	REQUIRE(file);
	MESSAGE("TemporaryFile: ", file->path());
	CHECK_FALSE(writer);
	const std::filesystem::path path{ file->path() };
	CHECK(std::filesystem::exists(path));
	{
		const auto blob = seir::Blob::from(*file);
		REQUIRE(blob);
		REQUIRE(blob->size() == 2 * data.size());
		CHECK_FALSE(std::memcmp(blob->data(), data.data(), data.size()));
		CHECK_FALSE(std::memcmp(static_cast<const std::byte*>(blob->data()) + data.size(), data.data(), data.size()));
	}
	file.reset();
	CHECK_FALSE(std::filesystem::exists(path));
}

TEST_CASE("TemporaryFile::commit({})")
{
	CHECK_FALSE(static_cast<bool>(seir::TemporaryWriter::commit({})));
}
