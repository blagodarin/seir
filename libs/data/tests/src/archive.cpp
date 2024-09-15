// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_data/archive.hpp>

#include <seir_base/buffer.hpp>
#include <seir_io/blob.hpp>
#include <seir_io/buffer_writer.hpp>
#include <seir_data/compression.hpp>
#include <seir_data/storage.hpp>
#include <seir_io/writer.hpp>

#include <algorithm>
#include <cstring>
#include <iterator>
#include <unordered_map>

#include <doctest/doctest.h>

TEST_CASE("Archiver")
{
	std::unordered_map<std::string, std::string> entries;
	std::generate_n(std::back_inserter(entries["digits.txt"]), 10 * 1024, [i = 0]() mutable { return static_cast<char>('0' + (i++ % 10)); });
	std::generate_n(std::back_inserter(entries["lowercase.txt"]), 26 * 1024, [i = 0]() mutable { return static_cast<char>('a' + (i++ % 26)); });
	std::generate_n(std::back_inserter(entries["uppercase.txt"]), 26 * 1024, [i = 0]() mutable { return static_cast<char>('A' + (i++ % 26)); });
	seir::Buffer buffer;
	uint64_t bufferSize = 0;
	{
		auto writer = seir::makeUnique<seir::Writer, seir::BufferWriter>(buffer, &bufferSize);
		seir::UniquePtr<seir::Archiver> archiver;
		SUBCASE("Compression::None")
		{
			archiver = seir::Archiver::create(std::move(writer), seir::Compression::None);
		}
#if SEIR_COMPRESSION_ZLIB
		SUBCASE("Compression::Zlib")
		{
			archiver = seir::Archiver::create(std::move(writer), seir::Compression::Zlib); // cppcheck-suppress[accessMoved]
		}
#endif
#if SEIR_COMPRESSION_ZSTD
		SUBCASE("Compression::Zstd")
		{
			archiver = seir::Archiver::create(std::move(writer), seir::Compression::Zstd); // cppcheck-suppress[accessMoved]
		}
#endif
		REQUIRE(archiver);
		for (const auto& [name, contents] : entries)
			REQUIRE(archiver->add(name, *seir::Blob::from(contents.data(), contents.size()), seir::CompressionLevel::Maximum));
		CHECK(archiver->finish());
	}
	seir::Storage storage{ seir::Storage::UseFileSystem::Never };
	{
		auto blob = seir::Blob::from(buffer.data(), static_cast<size_t>(bufferSize));
		REQUIRE(blob);
		REQUIRE(storage.attachArchive(std::move(blob)));
	}
	for (const auto& [name, contents] : entries)
	{
		const auto blob = storage.open(name);
		REQUIRE(blob);
		REQUIRE(blob->size() == contents.size());
		CHECK_FALSE(std::memcmp(blob->data(), contents.data(), contents.size()));
	}
}
