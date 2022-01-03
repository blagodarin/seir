// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_data/save_file.hpp>

#include <filesystem>
#include <fstream>

#include <doctest/doctest.h>

namespace
{
	void checkFile(const std::filesystem::path& path, std::string_view data)
	{
		REQUIRE(std::filesystem::exists(path));
		std::ifstream stream{ path, std::ios::ate | std::ios::binary };
		stream.exceptions(std::ios::badbit | std::ios::failbit);
		REQUIRE(stream.tellg() == data.size());
		stream.seekg(0, std::ios::beg);
		std::string buffer(data.size(), '?');
		stream.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
		REQUIRE(buffer == data);
	}

	void writeFile(const std::filesystem::path& path, std::string_view data)
	{
		std::ofstream stream{ path, std::ios::binary | std::ios::trunc };
		stream.exceptions(std::ios::badbit | std::ios::failbit);
		stream << data;
	}
}

TEST_CASE("SaveFile")
{
	const auto path = std::filesystem::temp_directory_path() / "test.seir.SaveFile";
	const std::string_view originalData{ "Hello world!" };
	::writeFile(path, originalData);
	auto file = seir::SaveFile::create(path.string());
	REQUIRE(file);
	::checkFile(path, originalData);
	const std::string_view modifiedData{ "Modified world!" };
	CHECK(file->reserve(modifiedData.size()));
	CHECK(file->size() == 0);
	REQUIRE(file->offset() == 0);
	REQUIRE(file->write(modifiedData.data(), modifiedData.size()));
	::checkFile(path, originalData);
	SUBCASE("SaveFile::commit")
	{
		REQUIRE(seir::SaveFile::commit(std::move(file)));
		CHECK_FALSE(file);
		::checkFile(path, modifiedData);
	}
	SUBCASE("SaveFile::~SaveFile")
	{
		CHECK(file->flush()); // Should successfully do nothing. Unfortunately we're unable to check that it actually does nothing.
		::checkFile(path, originalData);
		file.reset();
		::checkFile(path, originalData);
	}
	std::filesystem::remove(path);
}

TEST_CASE("SaveFile::commit({})")
{
	CHECK_FALSE(seir::SaveFile::commit({}));
}

TEST_CASE("SaveFile::create({})")
{
	CHECK_FALSE(static_cast<bool>(seir::SaveFile::create({})));
}

TEST_CASE("SaveFile::create(\".../\")")
{
	auto path = std::filesystem::temp_directory_path().string();
	if (constexpr auto separator = static_cast<char>(std::filesystem::path::preferred_separator); path.empty() || path.back() != separator)
		path += separator;
	CHECK_FALSE(static_cast<bool>(seir::SaveFile::create(std::move(path))));
}
