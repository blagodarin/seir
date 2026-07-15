// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_io/directory_view.hpp>

#include <seir_base/string_utils.hpp>

#include <algorithm>

namespace seir
{
	DirectoryView::DirectoryView(std::string_view pattern)
		: _pattern{ pattern }
	{
		reset(std::filesystem::current_path());
	}

	void DirectoryView::reset(const std::filesystem::path& path)
	{
		decltype(_entries) entries;
		try
		{
			if (path.has_parent_path() && path.parent_path() != path)
				entries.emplace_back("..", Entry::Type::Directory, path.parent_path());
			for (const auto& entry : std::filesystem::directory_iterator{ path })
			{
				auto filename = entry.path().filename().string();
				if (filename.starts_with('.'))
					continue;
				if (entry.is_directory())
					entries.emplace_back(std::move(filename), Entry::Type::Directory, entry.path());
				else if (entry.is_regular_file() && matchWildcard(filename, _pattern))
					entries.emplace_back(std::move(filename), Entry::Type::File, entry.path());
			}
		}
		catch (const std::filesystem::filesystem_error&)
		{
			return;
		}
		std::sort(path.has_parent_path() ? std::next(entries.begin()) : entries.begin(), entries.end(),
			[](const Entry& left, const Entry& right) {
				if (left.type != right.type)
					return left.type == Entry::Type::Directory;
				return left.name < right.name;
			});
		_path = path.string();
		_entries = std::move(entries);
	}
}
