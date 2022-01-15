// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_base/buffer.hpp>

#include <cstddef>
#include <vector>

#include <doctest/doctest.h>

namespace
{
	using Buffer = seir::Buffer<seir::CleanAllocator<seir::Allocator>>;

	void check(Buffer& buffer, const std::vector<int>& expected, bool allocated = true)
	{
		CHECK(buffer.capacity() >= expected.size());
		auto data = buffer.data();
		CHECK(std::as_const(buffer).data() == data);
		if (!expected.empty())
		{
			REQUIRE(data);
			for (size_t i = 0; i < expected.size(); ++i)
			{
				INFO('[', i, ']');
				CHECK(std::to_integer<int>(data[i]) == expected[i]);
			}
		}
		else if (!allocated)
			CHECK_FALSE(data);
	}
}

TEST_CASE("Buffer::Buffer(0)")
{
	Buffer buffer{ 0 };
	::check(buffer, {});
}

TEST_CASE("Buffer::Buffer(Buffer&&)")
{
	Buffer buffer{ 3 };
	::check(buffer, { 0, 0, 0 });
	for (size_t i = 0; i < 3; ++i)
		buffer.data()[i] = static_cast<std::byte>(i + 1);
	Buffer otherBuffer{ std::move(buffer) };
	::check(buffer, {}, false);
	::check(otherBuffer, { 1, 2, 3 });
}

TEST_CASE("Buffer::operator=(Buffer&&)")
{
	Buffer buffer{ 3 };
	::check(buffer, { 0, 0, 0 });
	for (size_t i = 0; i < 3; ++i)
		buffer.data()[i] = static_cast<std::byte>(i + 1);
	Buffer otherBuffer;
	::check(otherBuffer, {}, false);
	otherBuffer = std::move(buffer);
	::check(buffer, {}, true);
	::check(otherBuffer, { 1, 2, 3 });
}

TEST_CASE("Buffer::reserve()")
{
	Buffer buffer;
	::check(buffer, {}, false);
	buffer.reserve(3, 4);
	::check(buffer, { 0, 0, 0 });
	for (size_t i = 0; i < 3; ++i)
		buffer.data()[i] = static_cast<std::byte>(i + 1);
	buffer.reserve(4, 4);
	::check(buffer, { 1, 2, 3, 0 });
	buffer.reserve(3, 0);
	::check(buffer, { 1, 2, 3, 0 });
	buffer.reserve(5, 0);
	::check(buffer, { 0, 0, 0, 0, 0 });
}

TEST_CASE("swap(Buffer&, Buffer&)")
{
	Buffer first{ 3 };
	::check(first, { 0, 0, 0 });
	for (size_t i = 0; i < 3; ++i)
		first.data()[i] = static_cast<std::byte>(i + 1);
	Buffer second{ 4 };
	::check(second, { 0, 0, 0, 0 });
	for (size_t i = 0; i < 4; ++i)
		second.data()[i] = static_cast<std::byte>(i + 4);
	swap(first, second);
	::check(first, { 4, 5, 6, 7 });
	::check(second, { 1, 2, 3 });
}
