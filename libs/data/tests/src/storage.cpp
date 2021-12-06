// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_data/blob.hpp>
#include <seir_data/storage.hpp>

#include <doctest/doctest.h>

TEST_CASE("Storage::attach")
{
	seir::Storage storage;
	CHECK_FALSE(storage.open("absent"));
	CHECK_FALSE(storage.open("present"));
	storage.attach("present", seir::SharedPtr{ seir::Blob::from(&storage, sizeof storage) });
	CHECK_FALSE(storage.open("absent"));
	const auto blob = storage.open("present");
	REQUIRE(blob);
	CHECK(blob->data() == &storage);
	CHECK(blob->size() == sizeof storage);
}
