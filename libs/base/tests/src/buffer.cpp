// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_base/buffer.hpp>

#include <span>
#include <vector>

#include <doctest/doctest.h>

namespace
{
	void checkNotAllocated(seir::Buffer& buffer)
	{
		CHECK(buffer.capacity() == 0);
		CHECK_FALSE(buffer.data());
		CHECK(std::as_const(buffer).data() == buffer.data());
	}

	std::pair<std::byte*, size_t> checkAllocated(seir::Buffer& buffer, size_t capacity)
	{
		REQUIRE(buffer.capacity() >= capacity);
		REQUIRE(buffer.data());
		CHECK(std::as_const(buffer).data() == buffer.data());
		return { buffer.data(), buffer.capacity() };
	}

	void setValues(std::span<std::byte> data, unsigned char first)
	{
		for (size_t i = 0; i < data.size(); ++i)
			data[i] = static_cast<std::byte>(i + first);
	}

	void checkValues(std::span<const std::byte> data, unsigned char first)
	{
		for (size_t i = 0; i < data.size(); ++i)
		{
			INFO('[', i, ']');
			CHECK(data[i] == static_cast<std::byte>(i + first));
		}
	}

	void checkNotValues(std::span<const std::byte> data, unsigned char first)
	{
		size_t matchingBytes = 0;
		for (size_t i = 0; i < data.size(); ++i)
			if (data[i] == static_cast<std::byte>(i + first))
				++matchingBytes;
		CHECK(matchingBytes < data.size());
	}
}

TEST_CASE("Buffer::Buffer()")
{
	seir::Buffer buffer;
	::checkNotAllocated(buffer);
}

TEST_CASE("Buffer::Buffer(0)")
{
	seir::Buffer buffer{ 0 };
	::checkAllocated(buffer, 0);
}

TEST_CASE("Buffer::Buffer(Buffer&&)")
{
	constexpr size_t N = 127;
	seir::Buffer buffer{ N };
	const auto [data, capacity] = ::checkAllocated(buffer, N);
	::setValues({ data, N }, 1);
	seir::Buffer otherBuffer{ std::move(buffer) };
	CHECK_FALSE(buffer.data());
	REQUIRE(otherBuffer.data() == data);
	REQUIRE(otherBuffer.capacity() == capacity);
	::checkValues({ data, N }, 1);
}

TEST_CASE("Buffer::operator=(Buffer&&)")
{
	constexpr size_t N = 127;
	seir::Buffer buffer{ N };
	const auto [data, capacity] = ::checkAllocated(buffer, N);
	::setValues({ data, N }, 1);
	seir::Buffer otherBuffer;
	::checkNotAllocated(otherBuffer);
	otherBuffer = std::move(buffer);
	CHECK_FALSE(buffer.data());
	REQUIRE(otherBuffer.data() == data);
	REQUIRE(otherBuffer.capacity() == capacity);
	::checkValues({ data, N }, 1);
}

TEST_CASE("Buffer::reserve()")
{
	constexpr size_t N = 127;
	seir::Buffer buffer;
	::checkNotAllocated(buffer);
	buffer.reserve(N, N + 1);
	const auto [data, capacity] = ::checkAllocated(buffer, N);
	::setValues({ data, N }, 1);
	SUBCASE("newCapacity > capacity")
	{
		const auto newCapacity = capacity + 1;
		SUBCASE("preservedCapacity == N")
		{
			buffer.reserve(newCapacity, N);
			::checkAllocated(buffer, newCapacity);
			CHECK(buffer.data() != data);
			::checkValues({ buffer.data(), N }, 1);
		}
		SUBCASE("preservedCapacity > newCapacity")
		{
			buffer.reserve(newCapacity, newCapacity + 1);
			::checkAllocated(buffer, newCapacity);
			CHECK(buffer.data() != data);
			::checkValues({ buffer.data(), N }, 1);
		}
		SUBCASE("preservedCapacity == 0")
		{
			buffer.reserve(newCapacity, 0);
			::checkAllocated(buffer, newCapacity);
			REQUIRE(buffer.data() != data);
			::checkNotValues({ buffer.data(), N }, 1);
		}
	}
	SUBCASE("newCapacity == capacity")
	{
		buffer.reserve(capacity, 0);
		REQUIRE(buffer.data() == data);
		REQUIRE(buffer.capacity() == capacity);
		::checkValues({ buffer.data(), N }, 1);
	}
	SUBCASE("newCapacity < capacity")
	{
		buffer.reserve(capacity - 1, 0);
		REQUIRE(buffer.data() == data);
		REQUIRE(buffer.capacity() == capacity);
		::checkValues({ buffer.data(), N }, 1);
	}
}

TEST_CASE("swap(Buffer&, Buffer&)")
{
	constexpr size_t N1 = 31;
	seir::Buffer buffer1{ N1 };
	const auto [data1, capacity1] = ::checkAllocated(buffer1, N1);
	::setValues({ data1, N1 }, 1);

	constexpr size_t N2 = 63;
	seir::Buffer buffer2{ N2 };
	const auto [data2, capacity2] = ::checkAllocated(buffer2, N2);
	::setValues({ data2, N2 }, 127);

	swap(buffer1, buffer2);

	REQUIRE(buffer1.data() == data2);
	REQUIRE(buffer1.capacity() == capacity2);
	CHECK(std::as_const(buffer1).data() == data2);
	::checkValues({ data2, N2 }, 127);

	REQUIRE(buffer2.data() == data1);
	REQUIRE(buffer2.capacity() == capacity1);
	CHECK(std::as_const(buffer2).data() == data1);
	::checkValues({ data1, N1 }, 1);
}
