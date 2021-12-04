// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <filesystem>

namespace seir
{
	class Blob;
	template <class>
	class UniquePtr;

	// Creates a Blob that references a memory-mapped file.
	// This should be a member function of Blob, but <filesystem> is extremely slow to compile.
	[[nodiscard]] UniquePtr<Blob> openFile(const std::filesystem::path&);
}
