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
			const auto wrap = [](const std::string& value) { // To simplify visual comparison of problematic cases.
				const std::string separator(64, '-');
				return '\n' + separator + '\n' + value + separator + '\n';
			};
			CHECK(wrap(seir::StWriter::commit(std::move(pretty))) == wrap(expectedPretty));
			CHECK(wrap(seir::StWriter::commit(std::move(compact))) == wrap(expectedCompact));
		};
		SUBCASE("empty")
		{
			check([](seir::StWriter&) {
				; // Begin + End
			},
				"", R"()");
		}
		SUBCASE("keys")
		{
			check([](seir::StWriter& w) {
				w.addKey("key1"); // Begin + Key
				w.addKey("key2"); // Key + Key
				;                 // Key + End
			},
				"key1\n"
				"key2\n",
				R"(key1 key2)");
		}
		SUBCASE("values")
		{
			check([](seir::StWriter& w) {
				w.addKey("key1");     //
				w.addValue("value1"); // Key + Value
				w.addValue("value2"); // Value + Value
				w.addKey("key2");     // Value + Key
				w.addValue("value3"); //
				;                     // Value + End
			},
				"key1 \"value1\" \"value2\"\n"
				"key2 \"value3\"\n",
				R"(key1"value1""value2"key2"value3")");
		}
		SUBCASE("value contents")
		{
			check([](seir::StWriter& w) {
				w.addKey("empty");
				w.addValue("");
				w.addKey("tab");
				w.addValue("\t");
				w.addKey("lf");
				w.addValue("\n");
				w.addKey("cr");
				w.addValue("\r");
				w.addKey("backslash");
				w.addValue("\\");
				w.addKey("quote");
				w.addValue("\"");
			},
				"empty \"\"\n"
				"tab \"\t\"\n"
				"lf \"\n"
				"\"\n"
				"cr \"\r\"\n"
				"backslash \"\\\\\"\n"
				"quote \"\\\"\"\n",
				"empty\"\"tab\"\t\"lf\"\n\"cr\"\r\"backslash\"\\\\\"quote\"\\\"\"");
		}
		SUBCASE("lists without values")
		{
			// Key + ListBegin
			// ListBegin + ListBegin
			// ListBegin + ListEnd
			// ListEnd + End
			// ListEnd + Key
			// ListEnd + ListBegin
			// ListEnd + ListEnd
			check([](seir::StWriter& w) {
				w.addKey("key1"); //
				w.beginList();    // Key + ListBegin
				w.endList();      // ListBegin + ListEnd
				w.addKey("key2"); // ListEnd + Key
				w.beginList();    //
				w.endList();      //
				w.beginList();    // ListEnd + ListBegin
				w.beginList();    // ListBegin + ListBegin
				w.endList();      // [ ListBegin + ListEnd ]
				w.beginList();    // [ ListEnd + ListBegin ]
				w.beginList();    // [ ListBegin + ListBegin ]
				w.endList();      // [ [ ListBegin + ListEnd ] ]
				w.beginList();    // [ [ ListEnd + ListBegin ] ]
				w.beginList();    // [ [ ListBegin + ListBegin ] ]
				w.endList();      // [ [ [ ListBegin + ListEnd ] ] ]
				w.endList();      // [ [ ListEnd + ListEnd ] ]
				w.endList();      // [ ListEnd + ListEnd ]
				w.endList();      // ListEnd + ListEnd
				;                 // ListEnd + End
			},
				"key1 [\n"
				"]\n"
				"key2 [\n"
				"] [\n"
				"  [\n"
				"  ]\n"
				"  [\n"
				"    [\n"
				"    ]\n"
				"    [\n"
				"      [\n"
				"      ]\n"
				"    ]\n"
				"  ]\n"
				"]\n",
				R"(key1[]key2[][[][[][[]]]])");
		}
		SUBCASE("lists with values")
		{
			// Value + Value (in lists)
			// Value + ListBegin
			// Value + ListEnd
			// ListBegin + Value
			// ListEnd + Value
			check([](seir::StWriter& w) {
				w.addKey("key");   //
				w.addValue("1.1"); //
				w.beginList();     // Value + ListBegin
				w.addValue("2.1"); // ListBegin + Value
				w.addValue("2.2"); // [ Value + Value ]
				w.beginList();     // [ Value + ListBegin ]
				w.addValue("3.1"); // [ ListBegin + Value ]
				w.addValue("3.2"); // [ [ Value + Value ] ]
				w.beginList();     // [ [ Value + ListBegin ] ]
				w.addValue("4.1"); // [ [ ListBegin + Value ] ]
				w.endList();       // [ [ Value + ListEnd ] ]
				w.addValue("3.3"); // [ [ ListEnd + Value ] ]
				w.endList();       // [ Value + ListEnd ]
				w.addValue("2.3"); // [ ListEnd + Value ]
				w.endList();       // Value + ListEnd
				w.addValue("1.2"); // ListEnd + Value
			},
				"key \"1.1\" [\n"
				"  \"2.1\"\n"
				"  \"2.2\"\n"
				"  [\n"
				"    \"3.1\"\n"
				"    \"3.2\"\n"
				"    [\n"
				"      \"4.1\"\n"
				"    ]\n"
				"    \"3.3\"\n"
				"  ]\n"
				"  \"2.3\"\n"
				"] \"1.2\"\n",
				R"(key"1.1"["2.1""2.2"["3.1""3.2"["4.1"]"3.3"]"2.3"]"1.2")");
		}
		SUBCASE("objects without values")
		{
			// Key + Key (in objects)
			// Key + ObjectBegin
			// Key + ObjectEnd
			// ObjectBegin + Key
			// ObjectBegin + ObjectEnd
			// ObjectEnd + End
			// ObjectEnd + Key
			// ObjectEnd + ObjectBegin
			// ObjectEnd + ObjectEnd
			check([](seir::StWriter& w) {
				w.addKey("key11"); //
				w.beginObject();   // Key + ObjectBegin
				w.endObject();     // ObjectBegin + ObjectEnd
				w.beginObject();   // ObjectEnd + ObjectBegin
				w.addKey("key12"); // ObjectBegin + Key
				w.endObject();     // Key + ObjectEnd
				w.addKey("key13"); // ObjectEnd + Key
				w.beginObject();   //
				w.addKey("key20"); //
				w.addKey("key21"); // { Key + Key }
				w.beginObject();   // { Key + ObjectBegin }
				w.endObject();     // { ObjectBegin + ObjectEnd }
				w.beginObject();   // { ObjectEnd + ObjectBegin }
				w.addKey("key22"); // { ObjectBegin + Key }
				w.endObject();     // { Key + ObjectEnd }
				w.addKey("key23"); // { ObjectEnd + Key }
				w.beginObject();   //
				w.addKey("key30"); //
				w.addKey("key31"); // { { Key + Key } }
				w.beginObject();   // { { Key + ObjectBegin } }
				w.endObject();     // { { ObjectBegin + ObjectEnd } }
				w.beginObject();   // { { ObjectEnd + ObjectBegin } }
				w.addKey("key32"); // { { ObjectBegin + Key } }
				w.endObject();     // { { Key + ObjectEnd } }
				w.addKey("key33"); // { { ObjectEnd + Key } }
				w.beginObject();   //
				w.endObject();     //
				w.endObject();     // { ObjectEnd + ObjectEnd }
				w.endObject();     // ObjectEnd + ObjectEnd
				;                  // ObjectEnd + End
			},
				"key11 {\n"
				"} {\n"
				"  key12\n"
				"}\n"
				"key13 {\n"
				"  key20\n"
				"  key21 {\n"
				"  } {\n"
				"    key22\n"
				"  }\n"
				"  key23 {\n"
				"    key30\n"
				"    key31 {\n"
				"    } {\n"
				"      key32\n"
				"    }\n"
				"    key33 {\n"
				"    }\n"
				"  }\n"
				"}\n",
				R"(key11{}{key12}key13{key20 key21{}{key22}key23{key30 key31{}{key32}key33{}}})");
		}
		SUBCASE("objects with values")
		{
			// Key + Value (in objects)
			// Value + Key (in objects)
			// Value + Value (in objects)
			// Value + ObjectBegin
			// Value + ObjectEnd
			// ObjectEnd + Value
			check([](seir::StWriter& w) {
				w.addKey("level1");   //
				w.addValue("begin1"); //
				w.beginObject();      // Value + ObjectBegin
				w.addKey("key1");     //
				w.addValue("1.1");    // { Key + Value }
				w.addValue("1.2");    // { Value + Value }
				w.addKey("level2");   // { Value + Key }
				w.addValue("begin2"); //
				w.beginObject();      // { Value + ObjectBegin }
				w.addKey("key2");     //
				w.addValue("2.1");    // { { Key + Value } }
				w.addValue("2.2");    // { { Value + Value } }
				w.addKey("level3");   // { { Value + Key } }
				w.addValue("begin3"); //
				w.beginObject();      // { { Value + ObjectBegin } }
				w.endObject();        //
				w.addValue("end3");   // { { ObjectEnd + Value } }
				w.endObject();        // { Value + ObjectEnd }
				w.addValue("end2");   // { ObjectEnd + Value }
				w.endObject();        // Value + ObjectEnd
				w.addValue("end1");   // ObjectEnd + Value
			},
				"level1 \"begin1\" {\n"
				"  key1 \"1.1\" \"1.2\"\n"
				"  level2 \"begin2\" {\n"
				"    key2 \"2.1\" \"2.2\"\n"
				"    level3 \"begin3\" {\n"
				"    } \"end3\"\n"
				"  } \"end2\"\n"
				"} \"end1\"\n",
				R"(level1"begin1"{key1"1.1""1.2"level2"begin2"{key2"2.1""2.2"level3"begin3"{}"end3"}"end2"}"end1")");
		}
		// TODO: Test objects in lists and lists in objects.
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
