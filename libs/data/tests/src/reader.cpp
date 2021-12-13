// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_data/reader.hpp>

#include <array>
#include <cstdint>

#include <ostream>
#include <doctest/doctest.h>

TEST_CASE("Reader")
{
	SUBCASE("size() == 0")
	{
		const auto blob = seir::Blob::from(nullptr, 0);
		seir::Reader reader{ *blob };
		CHECK(reader.offset() == 0);
		CHECK(reader.size() == 0);
		SUBCASE("read()") { CHECK_FALSE(reader.read<uint8_t>()); }
		SUBCASE("seek(0)") { CHECK(reader.seek(0)); }
		SUBCASE("seek(1)") { CHECK_FALSE(reader.seek(1)); }
		SUBCASE("skip(0)") { CHECK(reader.skip(0)); }
		SUBCASE("skip(1)") { CHECK_FALSE(reader.skip(1)); }
		CHECK(reader.offset() == 0);
	}
	SUBCASE("size() > 0")
	{
		const std::array<char, 24> buffer{
			'0', '1', '2', '3', '4', '5', '6', '7',
			'8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
			'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n'
		};
		const auto blob = seir::Blob::from(buffer.data(), buffer.size());
		seir::Reader reader{ *blob };
		CHECK(reader.offset() == 0);
		CHECK(reader.size() == buffer.size());
		size_t expectedOffset = 0;
		SUBCASE("seek(0)")
		{
			CHECK(reader.seek(0));
			CHECK(reader.offset() == 0);
		}
		SUBCASE("seek(4) && seek(8)")
		{
			CHECK(reader.seek(4));
			CHECK(reader.offset() == 4);
			CHECK(reader.seek(8));
			CHECK(reader.offset() == 8);
			expectedOffset = 8;
		}
		SUBCASE("skip(4) && skip(8)")
		{
			CHECK(reader.skip(4));
			CHECK(reader.offset() == 4);
			CHECK(reader.skip(8));
			CHECK(reader.offset() == 12);
			expectedOffset = 12;
		}
		CHECK(reader.read<uint32_t>() == reinterpret_cast<const uint32_t*>(buffer.data() + expectedOffset));
		expectedOffset += 4;
		CHECK(reader.offset() == expectedOffset);
		CHECK(reader.read<uint16_t>() == reinterpret_cast<const uint16_t*>(buffer.data() + expectedOffset));
		expectedOffset += 2;
		CHECK(reader.offset() == expectedOffset);
		CHECK(reader.read<uint8_t>() == reinterpret_cast<const uint8_t*>(buffer.data() + expectedOffset));
		expectedOffset += 1;
		CHECK(reader.offset() == expectedOffset);
		CHECK_FALSE(reader.read<std::array<std::byte, buffer.size()>>());
		CHECK(reader.offset() == expectedOffset);
	}
}

TEST_CASE("Reader::readArray")
{
	const auto check = []<class T>(T) {
		const std::array<T, 2> buffer{ 1, 2 };
		for (size_t blobSize = 0; blobSize <= buffer.size() * sizeof(T); ++blobSize)
		{
			INFO("blobSize = ", blobSize);
			const auto blob = seir::Blob::from(buffer.data(), blobSize);
			for (size_t elementsToRead = 0; elementsToRead <= blobSize / sizeof(T) + 1; ++elementsToRead)
			{
				INFO("elementsToRead = ", elementsToRead);
				seir::Reader reader{ *blob };
				const auto expectedHeadSize = std::min(elementsToRead, blob->size() / sizeof(T));
				const auto head = reader.readArray<T>(elementsToRead);
				CHECK(head.data() == buffer.data());
				CHECK(head.size() == expectedHeadSize);
				CHECK(reader.offset() == expectedHeadSize * sizeof(T));
				if (elementsToRead <= blobSize / sizeof(T))
				{
					const auto expectedTailSize = blobSize / sizeof(T) - elementsToRead;
					const auto tail = reader.readArray<T>(expectedTailSize + 1);
					CHECK(tail.data() == buffer.data() + expectedHeadSize);
					CHECK(tail.size() == expectedTailSize);
					CHECK(reader.offset() == (expectedHeadSize + expectedTailSize) * sizeof(T));
				}
			}
		}
	};
	SUBCASE("int8_t") { check(int8_t{}); }
	SUBCASE("int16_t") { check(int16_t{}); }
}

TEST_CASE("Reader::readBlocks")
{
	const std::array<int8_t, 5> buffer{ 1, 2, 3, 4, 5 };
	for (size_t blockSize = 1; blockSize <= 2; ++blockSize)
	{
		INFO("blockSize = ", blockSize);
		for (size_t blobSize = 0; blobSize <= buffer.size(); ++blobSize)
		{
			INFO("blobSize = ", blobSize);
			const auto blob = seir::Blob::from(buffer.data(), blobSize);
			for (size_t blocksToRead = 0; blocksToRead <= blobSize / blockSize + 1; ++blocksToRead)
			{
				INFO("blocksToRead = ", blocksToRead);
				seir::Reader reader{ *blob };
				const auto expectedHeadSize = std::min(blocksToRead, blob->size() / blockSize);
				const auto [headData, headSize] = reader.readBlocks(blocksToRead, blockSize);
				CHECK(headData == buffer.data());
				CHECK(headSize == expectedHeadSize);
				CHECK(reader.offset() == expectedHeadSize * blockSize);
				if (blocksToRead <= blobSize / blockSize)
				{
					const auto expectedTailSize = blobSize / blockSize - blocksToRead;
					const auto [tailData, tailSize] = reader.readBlocks(expectedTailSize + 1, blockSize);
					CHECK(tailData == buffer.data() + expectedHeadSize * blockSize);
					CHECK(tailSize == expectedTailSize);
					CHECK(reader.offset() == (expectedHeadSize + expectedTailSize) * blockSize);
				}
			}
		}
	}
}

TEST_CASE("Reader::readLine")
{
	const auto strip = [](std::string_view text) {
		if (text.ends_with("\r\n"))
			return text.substr(0, text.size() - 2);
		if (text.ends_with('\r') || text.ends_with('\n'))
			return text.substr(0, text.size() - 1);
		return text;
	};
	SUBCASE("empty")
	{
		const std::string_view buffer{ "" };
		const auto blob = seir::Blob::from(buffer.data(), buffer.size());
		seir::Reader reader{ *blob };
		CHECK(reader.readLine().empty());
	}
	SUBCASE("newline")
	{
		std::string_view buffer;
		SUBCASE("windows") { buffer = "\r\n"; }
		SUBCASE("unix") { buffer = "\n"; }
		SUBCASE("mac") { buffer = "\r"; }
		const auto blob = seir::Blob::from(buffer.data(), buffer.size());
		seir::Reader reader{ *blob };
		const auto line = reader.readLine();
		CHECK(!line.empty());
		CHECK(strip(line) == "");
		CHECK(reader.readLine().empty());
	}
	SUBCASE("one line")
	{
		std::string_view buffer;
		SUBCASE("windows") { buffer = "text\r\n"; }
		SUBCASE("unix") { buffer = "text\n"; }
		SUBCASE("mac") { buffer = "text\r"; }
		SUBCASE("eof") { buffer = "text"; }
		const auto blob = seir::Blob::from(buffer.data(), buffer.size());
		seir::Reader reader{ *blob };
		CHECK(strip(reader.readLine()) == "text");
		CHECK(reader.readLine().empty());
	}
	SUBCASE("two lines")
	{
		std::string_view buffer;
		SUBCASE("windows") { buffer = "first\r\nsecond"; }
		SUBCASE("unix") { buffer = "first\nsecond"; }
		SUBCASE("mac") { buffer = "first\rsecond"; }
		const auto blob = seir::Blob::from(buffer.data(), buffer.size());
		seir::Reader reader{ *blob };
		CHECK(strip(reader.readLine()) == "first");
		CHECK(strip(reader.readLine()) == "second");
		CHECK(reader.readLine().empty());
	}
	SUBCASE("double newline")
	{
		std::string_view buffer;
		SUBCASE("windows-windows") { buffer = "\r\n\r\neof"; }
		SUBCASE("windows-unix") { buffer = "\r\n\neof"; }
		SUBCASE("windows-mac") { buffer = "\r\n\reof"; }
		SUBCASE("unix-windows") { buffer = "\n\r\neof"; }
		SUBCASE("unix-unix") { buffer = "\n\neof"; }
		SUBCASE("unix-mac") { buffer = "\n\reof"; }
		SUBCASE("mac-windows") { buffer = "\r\r\neof"; }
		SUBCASE("mac-mac") { buffer = "\r\reof"; }
		const auto blob = seir::Blob::from(buffer.data(), buffer.size());
		seir::Reader reader{ *blob };
		CHECK(strip(reader.readLine()).empty());
		CHECK(strip(reader.readLine()).empty());
		CHECK(reader.readLine() == "eof");
		CHECK(reader.readLine().empty());
	}
}
