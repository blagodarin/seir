// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_data/blob.hpp>
#include <seir_data/file.hpp>
#include <seir_data/storage.hpp>
#include "common.hpp"

#include <cstring>

#include <doctest/doctest.h>

TEST_CASE("Storage::attach")
{
	seir::Storage storage{ seir::Storage::UseFileSystem::Never };
	CHECK_FALSE(storage.open("absent"));
	CHECK_FALSE(storage.open("present"));
	storage.attach("present", seir::SharedPtr{ seir::Blob::from(&storage, sizeof storage) });
	CHECK_FALSE(storage.open("absent"));
	const auto blob = storage.open("present");
	REQUIRE(blob);
	CHECK(blob->data() == &storage);
	CHECK(blob->size() == sizeof storage);
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
			const auto blob = storage.open(thisExecutable.string());
			REQUIRE(blob);
			checkEqual(blob, file);
		}
		SUBCASE("attach")
		{
			storage.attach(thisExecutable.string(), dummy);
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
			const auto blob = storage.open(thisExecutable.string());
			REQUIRE(blob);
			checkEqual(blob, file);
		}
		SUBCASE("attach")
		{
			storage.attach(thisExecutable.string(), dummy);
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
			CHECK_FALSE(storage.open(thisExecutable.string()));
		}
		SUBCASE("attach")
		{
			storage.attach(thisExecutable.string(), dummy);
			const auto blob = storage.open(thisExecutable.string());
			REQUIRE(blob);
			checkEqual(blob, dummy);
		}
	}
}
