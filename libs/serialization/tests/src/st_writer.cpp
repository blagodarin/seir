// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_serialization/st_writer.hpp>

#include <functional>

#include <doctest/doctest.h>

TEST_CASE("StWriter")
{
	SUBCASE("positive")
	{
		const auto check = [](const std::function<void(seir::StWriter&)>& usage, const std::string& expectedPretty, const std::string& expectedCompact) {
			seir::StWriter compact{ seir::StWriter::Formatting::Compact };
			seir::StWriter pretty{ seir::StWriter::Formatting::Pretty };
			usage(compact);
			usage(pretty);
			CHECK(seir::StWriter::commit(std::move(compact)) == expectedCompact);
			CHECK(seir::StWriter::commit(std::move(pretty)) == expectedPretty);
		};
		SUBCASE("empty")
		{
			check([](seir::StWriter&) {}, "", R"()");
		}
		SUBCASE("keys and values")
		{
			check([](seir::StWriter& w) {
				w.addKey("key");
			},
				"key\n", R"(key)");
			check([](seir::StWriter& w) {
				w.addKey("key");
				w.addValue("value");
			},
				"key \"value\"\n", R"(key"value")");
			check([](seir::StWriter& w) {
				w.addKey("key");
				w.addValue("value1");
				w.addValue("value2");
			},
				"key \"value1\" \"value2\"\n", R"(key"value1""value2")");
			check([](seir::StWriter& w) {
				w.addKey("key1");
				w.addValue("value11");
				w.addValue("value12");
				w.addKey("key2");
			},
				"key1 \"value11\" \"value12\"\n"
				"key2\n",
				R"(key1"value11""value12"key2)");
		}
		SUBCASE("lists")
		{
			check([](seir::StWriter& w) {
				w.addKey("key");
				w.beginList();
				w.endList();
				w.addKey("key2");
				w.beginList();
				w.addValue("value21");
				w.addValue("value22");
				w.endList();
			},
				"key [\n"
				"]\n"
				"key2 [\n"
				"\t\"value21\"\n"
				"\t\"value22\"\n"
				"]\n",
				R"(key[]key2["value21""value22"])");
			check([](seir::StWriter& w) {
				w.addKey("key");
				w.addValue("value1");
				w.beginList();
				w.addValue("value2");
				w.endList();
				w.addValue("value3");
				w.addKey("key2");
			},
				"key \"value1\" [\n"
				"\t\"value2\"\n"
				"] \"value3\"\n"
				"key2\n",
				R"(key"value1"["value2"]"value3"key2)");
		}
		// TODO: Add nested lists tests.
		// TODO: Add object tests.
		// TODO: Add value escaping tests.
	}
	SUBCASE("negative")
	{
		const auto check = [](const std::function<void(seir::StWriter&)>& usage) {
			seir::StWriter compact{ seir::StWriter::Formatting::Compact };
			seir::StWriter pretty{ seir::StWriter::Formatting::Pretty };
			CHECK_THROWS_AS(usage(compact), seir::StWriter::UnexpectedToken);
			CHECK_THROWS_AS(usage(pretty), seir::StWriter::UnexpectedToken);
		};

		check([](seir::StWriter& w) { w.addValue("value"); });
		check([](seir::StWriter& w) { w.beginList(); });
		check([](seir::StWriter& w) { w.beginObject(); });
		check([](seir::StWriter& w) { w.endList(); });
		check([](seir::StWriter& w) { w.endObject(); });

		check([](seir::StWriter& w) { CHECK_NOTHROW(w.addKey("key")); w.endList(); });
		check([](seir::StWriter& w) { CHECK_NOTHROW(w.addKey("key")); w.endObject(); });

		check([](seir::StWriter& w) { CHECK_NOTHROW(w.addKey("key")); CHECK_NOTHROW(w.beginList()); w.addKey("key2"); });
		check([](seir::StWriter& w) { CHECK_NOTHROW(w.addKey("key")); CHECK_NOTHROW(w.beginList()); w.endObject(); });
		check([](seir::StWriter& w) { CHECK_NOTHROW(w.addKey("key")); CHECK_NOTHROW(w.beginList()); std::ignore = seir::StWriter::commit(std::move(w)); });

		check([](seir::StWriter& w) { CHECK_NOTHROW(w.addKey("key")); CHECK_NOTHROW(w.beginObject()); w.addValue("value"); });
		check([](seir::StWriter& w) { CHECK_NOTHROW(w.addKey("key")); CHECK_NOTHROW(w.beginObject()); w.beginList(); });
		check([](seir::StWriter& w) { CHECK_NOTHROW(w.addKey("key")); CHECK_NOTHROW(w.beginObject()); w.beginObject(); });
		check([](seir::StWriter& w) { CHECK_NOTHROW(w.addKey("key")); CHECK_NOTHROW(w.beginObject()); w.endList(); });
		check([](seir::StWriter& w) { CHECK_NOTHROW(w.addKey("key")); CHECK_NOTHROW(w.beginObject()); std::ignore = seir::StWriter::commit(std::move(w)); });
	}
}
