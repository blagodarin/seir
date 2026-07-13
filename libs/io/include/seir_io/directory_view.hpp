// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <filesystem>
#include <vector>

namespace seir
{
	class DirectoryView
	{
	public:
		struct Entry
		{
			enum class Type : bool
			{
				File,
				Directory,
			};

			std::string name;
			Type type = Type::File;
			std::filesystem::path path;
			std::string fullName = path.string();
		};

		DirectoryView(std::string_view pattern);

		[[nodiscard]] const std::string& path() const noexcept { return _path; }
		[[nodiscard]] const std::vector<Entry>& entries() const noexcept { return _entries; }
		void reset(const std::filesystem::path&);

	private:
		const std::string _pattern;
		std::string _path;
		std::vector<Entry> _entries;
	};
}
