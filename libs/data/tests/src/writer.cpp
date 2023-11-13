// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_data/writer.hpp>

#include <seir_data/blob.hpp>

#include <cstring>
#include <filesystem>

#include <doctest/doctest.h>

TEST_CASE("Writer")
{
	struct WriterTester final : public seir::Writer
	{
		bool _reserveResult = true;
		bool _writeResult = true;
		bool flush() noexcept override { return true; }

	private:
		bool reserveImpl(uint64_t) noexcept override { return _reserveResult; }
		bool writeImpl(uint64_t, const void*, size_t) noexcept override { return _writeResult; }
	};

	WriterTester tester;
	CHECK(tester.flush()); // To keep test source coverage at 100%.
	const auto check = [&tester](uint64_t size, uint64_t offset) {
		CHECK(tester.size() == size);
		CHECK(tester.offset() == offset);
	};
	check(0, 0);
	SUBCASE("reserve() == true")
	{
		CHECK(tester.reserve(7));
		check(0, 0);
	}
	SUBCASE("reserve() == false")
	{
		tester._reserveResult = false;
		CHECK_FALSE(tester.reserve(7));
		check(0, 0);
	}
	SUBCASE("write() == true")
	{
		CHECK(tester.write(nullptr, 7));
		check(7, 7);
		tester._writeResult = false;
		CHECK_FALSE(tester.write(nullptr, 13));
		check(7, 7);
	}
	SUBCASE("write() == false")
	{
		tester._writeResult = false;
		CHECK_FALSE(tester.write(nullptr, 7));
		check(0, 0);
	}
}

TEST_CASE("Writer::create(const std::string&)")
{
	const auto path = std::filesystem::temp_directory_path() / "test.txt";
	std::filesystem::remove(path);
	const std::string_view data{ "Hello world!\n" };
	{
		const auto writer = seir::Writer::create(path.string());
		REQUIRE(writer);
		REQUIRE(std::filesystem::exists(path));
		CHECK(writer->reserve(2 * data.size()));
		CHECK(writer->flush());
		CHECK(std::filesystem::file_size(path) == 0);
		REQUIRE(writer->write(data.data(), data.size()));
		REQUIRE(writer->write(data.data(), data.size()));
	}
	REQUIRE(std::filesystem::exists(path));
	CHECK(std::filesystem::file_size(path) == 2 * data.size());
	{
		const auto blob = seir::Blob::from(path.string());
		REQUIRE(blob);
		REQUIRE(blob->size() == 2 * data.size());
		CHECK_FALSE(std::memcmp(blob->data(), data.data(), data.size()));
		CHECK_FALSE(std::memcmp(static_cast<const std::byte*>(blob->data()) + data.size(), data.data(), data.size()));
	}
	std::filesystem::remove(path);
}

TEST_CASE("Writer::create({})")
{
	CHECK_FALSE(static_cast<bool>(seir::Writer::create(std::string{})));
}
