// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_data/archive.hpp>
#include <seir_data/blob.hpp>
#include <seir_data/compression.hpp>
#include <seir_data/file.hpp>
#include <seir_data/writer.hpp>

#include <unordered_map>

#include <doctest/doctest.h>

TEST_CASE("Archiver")
{
	std::unordered_map<std::string, std::string> entries;
	std::generate_n(std::back_inserter(entries["digits.txt"]), 10 * 1024, [i = 0]() mutable { return static_cast<char>('0' + (i++ % 10)); });
	std::generate_n(std::back_inserter(entries["lowercase.txt"]), 26 * 1024, [i = 0]() mutable { return static_cast<char>('a' + (i++ % 26)); });
	std::generate_n(std::back_inserter(entries["uppercase.txt"]), 26 * 1024, [i = 0]() mutable { return static_cast<char>('A' + (i++ % 26)); });
	const auto file = seir::TemporaryFile::create();
	REQUIRE(file);
	{
		auto writer = seir::createFileWriter(*file);
		REQUIRE(writer);
		seir::UniquePtr<seir::Archiver> archiver;
		SUBCASE("Compression::None")
		{
			archiver = seir::Archiver::create(std::move(writer), seir::Compression::None);
		}
#if SEIR_COMPRESSION_ZLIB
		SUBCASE("Compression::Zlib")
		{
			archiver = seir::Archiver::create(std::move(writer), seir::Compression::Zlib);
		}
#endif
#if SEIR_COMPRESSION_ZSTD
		SUBCASE("Compression::Zstd")
		{
			archiver = seir::Archiver::create(std::move(writer), seir::Compression::Zstd);
		}
#endif
		REQUIRE(archiver);
		for (const auto& entry : entries)
			REQUIRE(archiver->add(entry.first, *seir::Blob::from(entry.second.data(), entry.second.size()), seir::CompressionLevel::BestCompression));
		CHECK(archiver->finish());
	}
	// TODO: Test archive loading.
}