// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_data/blob.hpp>
#include <seir_data/compression.hpp>
#include <seir_data/file.hpp>
#include <seir_data/storage.hpp>
#include "common.hpp"

#include <algorithm>
#include <cstring>

#include <doctest/doctest.h>

TEST_CASE("Storage::attach")
{
	std::vector<uint8_t> contents;
	std::fill_n(std::back_inserter(contents), 256, uint8_t{ 42 });
	seir::Storage storage{ seir::Storage::UseFileSystem::Never };
	CHECK_FALSE(storage.open("absent"));
	CHECK_FALSE(storage.open("present"));
	SUBCASE("Compression::None")
	{
		storage.attach("present", seir::SharedPtr{ seir::Blob::from(contents.data(), contents.size()) });
	}
#if SEIR_COMPRESSION_ZLIB
	SUBCASE("Compression::Zlib")
	{
		const auto compressor = seir::Compressor::create(seir::Compression::Zlib);
		REQUIRE(compressor);
		REQUIRE(compressor->prepare(9));
		const std::string_view garbage = "garbage";
		seir::Buffer<uint8_t> buffer{ garbage.size() + compressor->maxCompressedSize(contents.size()) + garbage.size() };
		std::memcpy(buffer.data(), garbage.data(), garbage.size());
		const auto compressedSize = compressor->compress(buffer.data() + garbage.size(), buffer.capacity() - 2 * garbage.size(), contents.data(), contents.size());
		std::memcpy(buffer.data() + garbage.size() + compressedSize, garbage.data(), garbage.size());
		storage.attach("present", seir::SharedPtr{ seir::Blob::from(std::move(buffer)) }, garbage.size(), contents.size(), seir::Compression::Zlib, compressedSize);
	}
#endif
	CHECK_FALSE(storage.open("absent"));
	const auto blob = storage.open("present");
	REQUIRE(blob);
	REQUIRE(blob->size() == contents.size());
	CHECK_FALSE(std::memcmp(blob->data(), contents.data(), contents.size()));
}

TEST_CASE("Storage::open")
{
	const seir::SharedPtr file{ seir::openFile(thisExecutable) };
	REQUIRE(file);
	seir::SharedPtr dummy{ seir::Blob::from(&file, sizeof file) };
	const auto checkEqual = [](const seir::SharedPtr<seir::Blob>& left, const seir::SharedPtr<seir::Blob>& right) {
		REQUIRE(left->size() == right->size());
		CHECK_FALSE(std::memcmp(left->data(), right->data(), left->size()));
	};
	SUBCASE("UseFileSystem::AfterAttachments")
	{
		seir::Storage storage{ seir::Storage::UseFileSystem::AfterAttachments };
		SUBCASE("open")
		{
			CHECK_FALSE(storage.open("does/not/exist"));
			const auto blob = storage.open(thisExecutable.string());
			REQUIRE(blob);
			checkEqual(blob, file);
		}
		SUBCASE("attach")
		{
			storage.attach(thisExecutable.string(), dummy);
			CHECK_FALSE(storage.open("does/not/exist"));
			const auto blob = storage.open(thisExecutable.string());
			REQUIRE(blob);
			checkEqual(blob, dummy);
		}
	}
	SUBCASE("UseFileSystem::BeforeAttachments")
	{
		seir::Storage storage{ seir::Storage::UseFileSystem::BeforeAttachments };
		SUBCASE("open")
		{
			CHECK_FALSE(storage.open("does/not/exist"));
			const auto blob = storage.open(thisExecutable.string());
			REQUIRE(blob);
			checkEqual(blob, file);
		}
		SUBCASE("attach")
		{
			storage.attach(thisExecutable.string(), dummy);
			CHECK_FALSE(storage.open("does/not/exist"));
			const auto blob = storage.open(thisExecutable.string());
			REQUIRE(blob);
			checkEqual(blob, file);
		}
	}
	SUBCASE("UseFileSystem::Never")
	{
		seir::Storage storage{ seir::Storage::UseFileSystem::Never };
		SUBCASE("open")
		{
			CHECK_FALSE(storage.open("does/not/exist"));
			CHECK_FALSE(storage.open(thisExecutable.string()));
		}
		SUBCASE("attach")
		{
			storage.attach(thisExecutable.string(), dummy);
			CHECK_FALSE(storage.open("does/not/exist"));
			const auto blob = storage.open(thisExecutable.string());
			REQUIRE(blob);
			checkEqual(blob, dummy);
		}
	}
}
