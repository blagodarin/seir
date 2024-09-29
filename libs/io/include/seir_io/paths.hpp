// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <optional>
#include <string>

namespace seir
{
	// Constructs a path to a screenshot, typically:
	// - "C:\Users\{username}\Pictures\Screenshots\{relativePath}" on Windows.
	// - "/home/{username}/{relativePath}" on Linux.
	std::optional<std::string> makeScreenshotPath(std::string_view relativePath);

	// Constructs a path to user-specific data that can be shared across devices
	// (progress, settings, etc), typically:
	// - "C:\Users\{username}\AppData\Roaming\{relativePath}" on Windows;
	// - "/home/{username}/.local/share/{relativePath}" on Linux.
	std::optional<std::string> makeUserDataPath(std::string_view relativePath);

	// Constructs a path to user-specific data that can't (or shouldn't) be shared across devices
	// (like caches, logs, etc), typically:
	// - "C:\Users\{username}\AppData\Local\{relativePath}" on Windows;
	// - "/home/{username}/.local/state/{relativePath}" on Linux.
	std::optional<std::string> makeUserStatePath(std::string_view relativePath);
}
