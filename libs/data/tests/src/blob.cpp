// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_data/blob.hpp>

#include <seir_base/endian.hpp>

#include "main.hpp"

#include <doctest/doctest.h>

TEST_CASE("Blob::from")
{
	const auto self = seir::Blob::from(thisExecutable);
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
