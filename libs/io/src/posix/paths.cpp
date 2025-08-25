// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_io/paths.hpp>

#include <cassert>
#include <cerrno>
#include <cstdlib>

#include <cstdio>     // perror
#include <sys/stat.h> // mkdir

// If HOME is not set, we can get user directory from getpwuid_r(getuid(), ...),
// but we've yet to find such an environment first.

namespace
{
	void cleanPath(std::string& path) noexcept
	{
		size_t written = 0;
		char last = '\0';
		for (size_t i = 0, end = path.size(); i < end; ++i)
		{
			const auto next = path[i];
			if (next != '/' || last != '/')
				path[written++] = next;
			last = next;
		}
		path.resize(written);
	}

	bool createDirectories(std::string& path) noexcept
	{
		assert(!path.empty());
		for (auto i = path.data() + 1; *i; ++i)
		{
			if (*i != '/')
				continue;
			*i = '\0';
			if (::mkdir(path.data(), 0700) != 0 && errno != EEXIST)
			{
				::perror("mkdir");
				return false;
			}
			*i = '/';
		}
		return true;
	}

	std::optional<std::string> buildPath(std::string_view prefix, std::string_view infix, std::string_view suffix)
	{
		std::string path;
		path.reserve(prefix.size() + infix.size() + suffix.size());
		path.assign(prefix).append(infix).append(suffix);
		::cleanPath(path);
		return ::createDirectories(path) ? std::optional{ path } : std::nullopt;
	}
}

namespace seir
{
	using namespace std::string_view_literals;

	std::optional<std::string> makeScreenshotPath(std::string_view relativePath)
	{
		if (const char* home = std::getenv("HOME"))
			return ::buildPath(home, "/"sv, relativePath);
		return {};
	}

	std::optional<std::string> makeUserDataPath(std::string_view relativePath)
	{
		if (const char* xdgDataHome = std::getenv("XDG_DATA_HOME"))
			return ::buildPath(xdgDataHome, "/"sv, relativePath);
		if (const char* home = std::getenv("HOME"))
			return ::buildPath(home, "/.local/share/"sv, relativePath);
		return {};
	}

	std::optional<std::string> makeUserStatePath(std::string_view relativePath)
	{
		if (const char* xdgDataHome = std::getenv("XDG_STATE_HOME"))
			return ::buildPath(xdgDataHome, "/"sv, relativePath);
		if (const char* home = std::getenv("HOME"))
			return ::buildPath(home, "/.local/state/"sv, relativePath);
		return {};
	}
}
