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

	//
	class TemporaryFile
	{
	public:
		// Creates a temporary file.
		// The file is deleted when the object is destroyed.
		[[nodiscard]] static UniquePtr<TemporaryFile> create();

		virtual ~TemporaryFile() noexcept = default;
	};

	// Creates a Blob that references a memory-mapped file.
	[[nodiscard]] SharedPtr<Blob> createFileBlob(const std::string&);
	[[nodiscard]] SharedPtr<Blob> createFileBlob(TemporaryFile&);

	// Creates a Writer thet writes to the specified file.
	[[nodiscard]] UniquePtr<Writer> createFileWriter(const std::string&);
	[[nodiscard]] UniquePtr<Writer> createFileWriter(TemporaryFile&);
}
