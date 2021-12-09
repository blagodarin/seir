// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_base/buffer.hpp>
#include <seir_data/writer.hpp>

#include <vector>

#include <doctest/doctest.h>

TEST_CASE("Writer::from(Buffer&)")
{
	seir::Buffer<std::byte> buffer;
	CHECK(buffer.capacity() == 0);
	const auto writer = seir::Writer::to(buffer);
	REQUIRE(writer);
	CHECK(writer->reserve(0));
	CHECK(buffer.capacity() == 0);
	CHECK(writer->reserve(1));
	CHECK(buffer.capacity() > 0);
	const auto check = [&buffer, &writer](const std::vector<uint8_t>& data, uint64_t offset) {
		CHECK(buffer.capacity() >= data.size());
		CHECK(writer->size() == data.size());
		CHECK(writer->offset() == offset);
	};
	check({}, 0);
	REQUIRE(writer->write(uint32_t{ 0x01010101 }));
	check({ 0x01, 0x01, 0x01, 0x01 }, 4);
	REQUIRE(writer->write(uint16_t{ 0x0202 }));
	check({ 0x01, 0x01, 0x01, 0x01, 0x02, 0x02 }, 6);
	REQUIRE(writer->seek(2));
	check({ 0x01, 0x01, 0x01, 0x01, 0x02, 0x02 }, 2);
	REQUIRE(writer->write(uint16_t{ 0x0303 }));
	check({ 0x01, 0x01, 0x03, 0x03, 0x02, 0x02 }, 4);
	const auto capacity = buffer.capacity();
	CHECK(writer->reserve(buffer.capacity() - writer->offset()));
	CHECK(buffer.capacity() == capacity);
	check({ 0x01, 0x01, 0x03, 0x03, 0x02, 0x02 }, 4);
	CHECK(writer->reserve(buffer.capacity() - writer->offset() + 1));
	CHECK(buffer.capacity() > capacity);
	check({ 0x01, 0x01, 0x03, 0x03, 0x02, 0x02 }, 4);
	CHECK_FALSE(writer->seek(7));
	CHECK(writer->offset() == 4);
	CHECK(writer->seek(6));
	CHECK(writer->offset() == 6);
}
