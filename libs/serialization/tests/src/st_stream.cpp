// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_serialization/st_stream.hpp>

#include <seir_io/blob.hpp>

#include <ostream>
#include <doctest/doctest.h>

namespace
{
	class StreamTester
	{
	public:
		StreamTester(std::string_view text)
			: _reader{ seir::Blob::from(text.data(), text.size()) } {}
		seir::StStream* operator->() noexcept { return &_stream; }

	private:
		seir::StReader _reader;
		seir::StStream _stream{ _reader };
	};
}

TEST_CASE("StStream")
{
	StreamTester stream{ "key \"value\"" };
	std::string_view text;

	CHECK_FALSE(stream->tryEnd());
	REQUIRE_THROWS_AS(text = stream->value(), seir::StStreamError);
	REQUIRE_NOTHROW(text = stream->key());
	CHECK(text == "key");

	CHECK_FALSE(stream->tryEnd());
	REQUIRE_THROWS_AS(text = stream->key(), seir::StStreamError);
	REQUIRE_NOTHROW(text = stream->value());
	CHECK(text == "value");

	CHECK(stream->tryEnd());
	CHECK(stream->tryEnd());
}
