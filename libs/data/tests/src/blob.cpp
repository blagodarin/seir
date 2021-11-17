// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_data/blob.hpp>

#include "main.hpp"

#include <doctest/doctest.h>

TEST_CASE("Blob::from")
{
	const auto self = seir::Blob::from(thisExecutable);
	REQUIRE(self);
#ifdef _WIN32
	REQUIRE(self->size() > 2);
	CHECK(static_cast<const char*>(self->data())[0] == 'M');
	CHECK(static_cast<const char*>(self->data())[1] == 'Z');
#endif
}
