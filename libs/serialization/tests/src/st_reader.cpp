// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_serialization/st_reader.hpp>

#include <seir_base/int_utils.hpp>
#include <seir_io/blob.hpp>

#include <ostream>
#include <vector>

#include <doctest/doctest.h>

namespace seir
{
	[[nodiscard]] constexpr bool operator==(const StToken& a, const StToken& b) noexcept
	{
		return a.line() == b.line() && a.column() == b.column() && a.type() == b.type() && a.text() == b.text();
	}

	inline std::ostream& operator<<(std::ostream& stream, const StToken& token)
	{
		switch (token.type())
		{
		case StToken::Type::Key: stream << "Key"; break;
		case StToken::Type::Value: stream << "Value"; break;
		case StToken::Type::ListBegin: stream << "ListBegin"; break;
		case StToken::Type::ListEnd: stream << "ListEnd"; break;
		case StToken::Type::ObjectBegin: stream << "ObjectBegin"; break;
		case StToken::Type::ObjectEnd: stream << "ObjectEnd"; break;
		case StToken::Type::End: stream << "End"; break;
		case StToken::Type::Error: stream << "Error"; break;
		}
		return stream << "(" << token.line() << "," << token.column() << ",\"" << token.text() << "\")";
	}
}

TEST_CASE("StReader::StReader")
{
	seir::StReader reader{ {} };
	const seir::StToken end{ 1, 1, seir::StToken::Type::End, "" };
	CHECK(reader.read() == end);
	CHECK(reader.read() == end);
}

TEST_CASE("StReader::read")
{
	const auto check = [](std::string_view data, const std::vector<seir::StToken>& tokens) {
		seir::StReader reader{ seir::Blob::from(data.data(), data.size()) };
		for (const auto& token : tokens)
		{
			CHECK(reader.read() == token);
			if (token.type() == seir::StToken::Type::End || token.type() == seir::StToken::Type::Error)
				CHECK(reader.read() == token);
		}
	};

	const auto end = [](size_t line, size_t column) { return seir::StToken{ line, column, seir::StToken::Type::End, "" }; };
	const auto error = [](size_t line, size_t column) { return seir::StToken{ line, column, seir::StToken::Type::Error, "" }; };
	const auto listBegin = [](size_t line, size_t column) { return seir::StToken{ line, column, seir::StToken::Type::ListBegin, "[" }; };
	const auto listEnd = [](size_t line, size_t column) { return seir::StToken{ line, column, seir::StToken::Type::ListEnd, "]" }; };
	const auto key = [](size_t line, size_t column, std::string_view text) { return seir::StToken{ line, column, seir::StToken::Type::Key, text }; };
	const auto objectBegin = [](size_t line, size_t column) { return seir::StToken{ line, column, seir::StToken::Type::ObjectBegin, "{" }; };
	const auto objectEnd = [](size_t line, size_t column) { return seir::StToken{ line, column, seir::StToken::Type::ObjectEnd, "}" }; };
	const auto value = [](size_t line, size_t column, std::string_view text) { return seir::StToken{ line, column, seir::StToken::Type::Value, text }; };

	SUBCASE("spaces")
	{
		SUBCASE("ok")
		{
			check("", { end(1, 1) });
			check(" ", { end(1, 2) });
			check("  ", { end(1, 3) });
			check("\t", { end(1, 2) });
			check("\n", { end(2, 1) });
			check("\r", { end(2, 1) });
			check("\r\n", { end(2, 1) });
			check("\n\r", { end(3, 1) });
		}
		SUBCASE("errors")
		{
			using namespace std::literals::string_literals;
			check("\0"s, { error(1, 1) });
			check("\xff", { error(1, 1) });
		}
	}
	SUBCASE("keys")
	{
		SUBCASE("ok")
		{
			check(
				"one",
				{
					key(1, 1, "one"),
					end(1, 4),
				});
			check(
				"one two",
				{
					key(1, 1, "one"),
					key(1, 5, "two"),
					end(1, 8),
				});
			check(
				"one\n"
				"two",
				{
					key(1, 1, "one"),
					key(2, 1, "two"),
					end(2, 4),
				});
		}
		SUBCASE("errors")
		{
			check("1", { error(1, 1) });
		}
	}
	SUBCASE("values")
	{
		SUBCASE("ok")
		{
			check(
				"one \"two\"",
				{
					key(1, 1, "one"),
					value(1, 5, "two"),
					end(1, 10),
				});
			check(
				"one\n"
				"  \"two\"",
				{
					key(1, 1, "one"),
					value(2, 3, "two"),
					end(2, 8),
				});
			check(
				"one \"two\" \"three\"",
				{
					key(1, 1, "one"),
					value(1, 5, "two"),
					value(1, 11, "three"),
					end(1, 18),
				});
			check(
				"one \"two\"\n"
				"  \"three\"",
				{
					key(1, 1, "one"),
					value(1, 5, "two"),
					value(2, 3, "three"),
					end(2, 10),
				});
			check(
				"one \"two\"\n"
				"three \"four\"",
				{
					key(1, 1, "one"),
					value(1, 5, "two"),
					key(2, 1, "three"),
					value(2, 7, "four"),
					end(2, 13),
				});
		}
		SUBCASE("errors")
		{
			using namespace std::literals::string_literals;
			check("\"", { error(1, 1) });
			check("key\"", { key(1, 1, "key"), error(1, 5) });
			check("key\"\0"s, { key(1, 1, "key"), error(1, 5) });
			check("key\"\n", { key(1, 1, "key"), error(1, 5) });
			check("key\"\r", { key(1, 1, "key"), error(1, 5) });
		}
	}
	SUBCASE("lists")
	{
		SUBCASE("ok")
		{
			check(R"(one[]["two"]["three""four"[]["five"]])",
				{
					key(1, 1, "one"),
					listBegin(1, 4),
					listEnd(1, 5),
					listBegin(1, 6),
					value(1, 7, "two"),
					listEnd(1, 12),
					listBegin(1, 13),
					value(1, 14, "three"),
					value(1, 21, "four"),
					listBegin(1, 27),
					listEnd(1, 28),
					listBegin(1, 29),
					value(1, 30, "five"),
					listEnd(1, 36),
					listEnd(1, 37),
					end(1, 38),
				});
			check(
				"one [\n"        // 1
				"] [\n"          // 2
				"  \"two\"\n"    // 3
				"] [\n"          // 4
				"  \"three\"\n"  // 5
				"  \"four\" [\n" // 6
				"  ] [\n"        // 7
				"    \"five\"\n" // 8
				"  ]\n"          // 9
				"]\n",           // 10
				{
					key(1, 1, "one"),
					listBegin(1, 5),
					listEnd(2, 1),
					listBegin(2, 3),
					value(3, 3, "two"),
					listEnd(4, 1),
					listBegin(4, 3),
					value(5, 3, "three"),
					value(6, 3, "four"),
					listBegin(6, 10),
					listEnd(7, 3),
					listBegin(7, 5),
					value(8, 5, "five"),
					listEnd(9, 3),
					listEnd(10, 1),
					end(11, 1),
				});
		}
		SUBCASE("errors")
		{
			check("[", { error(1, 1) });
			check("]", { error(1, 1) });
			check("key]", { key(1, 1, "key"), error(1, 4) });
			check("key[key", { key(1, 1, "key"), listBegin(1, 4), error(1, 5) });
			check("key[]]", { key(1, 1, "key"), listBegin(1, 4), listEnd(1, 5), error(1, 6) });
		}
	}
	SUBCASE("objects")
	{
		SUBCASE("ok")
		{
			check("one{}{two}{three four{}{five}}",
				{
					key(1, 1, "one"),
					objectBegin(1, 4),
					objectEnd(1, 5),
					objectBegin(1, 6),
					key(1, 7, "two"),
					objectEnd(1, 10),
					objectBegin(1, 11),
					key(1, 12, "three"),
					key(1, 18, "four"),
					objectBegin(1, 22),
					objectEnd(1, 23),
					objectBegin(1, 24),
					key(1, 25, "five"),
					objectEnd(1, 29),
					objectEnd(1, 30),
					end(1, 31),
				});
			check(
				"one {\n"    // 1
				"} {\n"      // 2
				"  two\n"    // 3
				"} {\n"      // 4
				"  three\n"  // 5
				"  four {\n" // 6
				"  } {\n"    // 7
				"    five\n" // 8
				"  }\n"      // 9
				"}\n",       // 10
				{
					key(1, 1, "one"),
					objectBegin(1, 5),
					objectEnd(2, 1),
					objectBegin(2, 3),
					key(3, 3, "two"),
					objectEnd(4, 1),
					objectBegin(4, 3),
					key(5, 3, "three"),
					key(6, 3, "four"),
					objectBegin(6, 8),
					objectEnd(7, 3),
					objectBegin(7, 5),
					key(8, 5, "five"),
					objectEnd(9, 3),
					objectEnd(10, 1),
					end(11, 1),
				});
		}
		SUBCASE("errors")
		{
			check("{", { error(1, 1) });
			check("}", { error(1, 1) });
			check("key}", { key(1, 1, "key"), error(1, 4) });
			check("key{[", { key(1, 1, "key"), objectBegin(1, 4), error(1, 5) });
			check("key{]", { key(1, 1, "key"), objectBegin(1, 4), error(1, 5) });
			check("key{{", { key(1, 1, "key"), objectBegin(1, 4), error(1, 5) });
			check("key{}}", { key(1, 1, "key"), objectBegin(1, 4), objectEnd(1, 5), error(1, 6) });
		}
	}
	SUBCASE("comments")
	{
		SUBCASE("ok")
		{
			check("//", { end(1, 3) });
			check("//comment", { end(1, 10) });
			check("//\n", { end(2, 1) });
			check("//comment\n", { end(2, 1) });
			check("//comment\nkey", { key(2, 1, "key"), end(2, 4) });
			check("key//", { key(1, 1, "key"), end(1, 6) });
			check("key//comment", { key(1, 1, "key"), end(1, 13) });
		}
		SUBCASE("errors")
		{
			check("/", { error(1, 1) });
			check("/comment", { error(1, 1) });
		}
	}
}
