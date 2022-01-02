// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <string>

namespace seir
{
	class Blob;
	template <class>
	class SharedPtr;
	template <class>
	class UniquePtr;
	class Writer;

	// Creates a Blob that references a memory-mapped file.
	[[nodiscard]] SharedPtr<Blob> createFileBlob(const std::string&);

	// Creates a Writer thet writes to the specified file.
	[[nodiscard]] UniquePtr<Writer> createFileWriter(const std::string&);
}
