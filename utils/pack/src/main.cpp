// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_data/archive.hpp>
#include <seir_data/blob.hpp>
#include <seir_data/compression.hpp>
#include <seir_data/writer.hpp>
#include <seir_serialization/st_stream.hpp>
#include <seir_u8main/u8main.hpp>

#include <cstring>
#include <iostream>
#include <string>
#include <vector>

namespace
{
	int usage()
	{
		std::cerr
			<< "Usage:\n"
			<< "  seir_pack INDEX PACKAGE\n"
			<< "  seir_pack --touch INDEX\n";
		return 1;
	}

	template <typename... Args>
	void check(bool condition, Args&&... args)
	{
		if (!condition)
			throw std::runtime_error{ std::forward<Args>(args)... };
	}

	struct Index
	{
		struct FileGroup
		{
			seir::CompressionLevel _compressionLevel = seir::CompressionLevel::None;
			std::vector<std::string> _files;
		};
		seir::Compression _compression = seir::Compression::None;
		std::vector<FileGroup> _groups;
	};

	Index readIndex(const std::string& path)
	{
		const auto blob = seir::Blob::from(path);
		check(static_cast<bool>(blob), "Bad index file");
		Index result;
		seir::StReader reader{ blob };
		seir::StStream stream{ reader };
		if (stream.tryKey("compressor"))
		{
			check(stream.value() == "zlib", "Bad compression algorithm");
			result._compression = seir::Compression::Zlib;
		}
		while (!stream.tryEnd())
		{
			stream.key("files");
			auto& group = result._groups.emplace_back();
			if (stream.tryBeginObject())
			{
				stream.key("compression");
				if (const auto level = stream.value(); level == "minimum")
					group._compressionLevel = seir::CompressionLevel::Minimum;
				else if (level == "default")
					group._compressionLevel = seir::CompressionLevel::Default;
				else if (level == "maximum")
					group._compressionLevel = seir::CompressionLevel::Maximum;
				else
					check(level == "none", "Bad compression level");
				stream.endObject();
			}
			stream.beginList();
			while (!stream.tryEndList())
				group._files.emplace_back(std::string{ stream.value() });
		}
		return result;
	}

	auto toPath(const char* path) { return std::filesystem::path{ reinterpret_cast<const char8_t*>(path) }; }
	auto toPath(const std::string& path) { return ::toPath(path.c_str()); };
}

int u8main(int argc, char** argv)
{
	if (argc < 2)
		return usage();
	try
	{
		if (!std::strcmp(argv[1], "--touch"))
		{
			if (argc != 3)
				return usage();

			auto dataTimestamp = std::filesystem::file_time_type::min();
			const auto index = readIndex(argv[2]);
			for (const auto& group : index._groups)
				for (const auto& file : group._files)
					dataTimestamp = std::max(dataTimestamp, std::filesystem::last_write_time(::toPath(file)));

			const auto indexPath = ::toPath(argv[2]);
			if (dataTimestamp > std::filesystem::last_write_time(indexPath))
				std::filesystem::last_write_time(indexPath, dataTimestamp);
		}
		else
		{
			if (argc != 3)
				return usage();
			const auto index = readIndex(argv[1]);
			const auto packagePath = ::toPath(argv[2]);
			auto fileWriter = seir::Writer::create(packagePath);
			if (!fileWriter)
			{
				std::cerr << "ERROR: Unable to open " << packagePath << " for writing\n";
				return 1;
			}
			auto packageWriter = seir::Archiver::create(std::move(fileWriter), index._compression);
			bool failed = false;
			std::cerr << "Writing " << packagePath << "...\n";
			for (const auto& group : index._groups)
				for (const auto& file : group._files)
					if (const auto blob = seir::Blob::from(file); blob)
					{
						std::cerr << " >> " << file << '\n';
						packageWriter->add(file, *blob, group._compressionLevel);
					}
					else
					{
						std::cerr << " !! " << file << '\n';
						failed = true;
						break;
					}
			if (failed || !packageWriter->finish())
			{
				std::cerr << "ERROR: Unable to write " << packagePath << '\n';
				packageWriter.reset();
				std::filesystem::remove(packagePath);
				return 1;
			}
			std::cerr << "Done!\n";
		}
	}
	catch (const std::runtime_error& e)
	{
		std::cerr << e.what() << '\n';
		return 1;
	}
	return 0;
}
