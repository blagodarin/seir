// This file is part of the Yttrium toolkit.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_data/blob.hpp>
#include <seir_data/writer.hpp>
#include <seir_u8main/u8main.hpp>

#include <cstring>
#include <iostream>
#include <span>

#include <fmt/core.h> // std::format_to isn't available on current macOS CI.

namespace
{
	int usage()
	{
		std::cerr
			<< "Usage:\n"
			<< "  seir_embed --string INPUT OUTPUT\n"
			<< "  seir_embed --uint8 INPUT OUTPUT\n";
		return 1;
	}

	void writeString(std::string& output, std::span<const char> input)
	{
		output += '"';
		bool newline = false; // To prevent "" at the end of newline-terminated files.
		bool hex = false;
		for (const auto c : input)
		{
			if (newline)
			{
				output += "\"\n\"";
				newline = false;
			}
			switch (c)
			{
			case '\0': output += "\\0"; break;
			case '\a': output += "\\a"; break;
			case '\b': output += "\\b"; break;
			case '\t': output += "\\t"; break;
			case '\n':
				output += "\\n";
				newline = true;
				break;
			case '\v': output += "\\v"; break;
			case '\f': output += "\\f"; break;
			case '\r': output += "\\r"; break;
			case '"': output += "\\\""; break;
			case '\\': output += "\\\\"; break;
			default:
				if (static_cast<unsigned char>(c) < 0x20)
				{
					fmt::format_to(std::back_inserter(output), "\\x{:02x}", c);
					hex = true;
					continue;
				}
				else
				{
					if (hex && ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')))
						output += "\"\"";
					output += c;
				}
			}
			hex = false;
		}
		output += "\"\n";
	}

	void writeUint8(std::string& output, std::span<const char> input)
	{
		constexpr size_t maxLineLength = 120;
		constexpr size_t maxItemLength = 4; // "255,"
		for (auto lineStart = output.size(); const auto c : input)
		{
			if (const auto currentOffset = output.size(); currentOffset - lineStart >= maxLineLength - maxItemLength)
			{
				output += '\n';
				lineStart = currentOffset;
			}
			fmt::format_to(std::back_inserter(output), "{:d},", static_cast<unsigned char>(c));
		}
		output += '\n';
	}
}

int u8main(int argc, char** argv)
{
	if (argc != 4)
		return usage();
	auto write = writeString;
	if (std::strcmp(argv[1], "--uint8") == 0)
		write = writeUint8;
	else if (std::strcmp(argv[1], "--string") != 0)
		return usage();
	const auto input = seir::Blob::from(argv[2]);
	if (!input)
	{
		std::cerr << "ERROR: Unable to open " << argv[2] << '\n';
		return 1;
	}
	if (const auto writer = seir::Writer::create(std::string{ argv[3] }); !writer)
	{
		std::cerr << "ERROR: Unable to open " << argv[3] << '\n';
		return 1;
	}
	else
	{
		std::string outputBuffer;
		outputBuffer.reserve(input->size());
		write(outputBuffer, std::span{ static_cast<const char*>(input->data()), input->size() });
		writer->write(outputBuffer.data(), outputBuffer.size());
	}
	return 0;
}
