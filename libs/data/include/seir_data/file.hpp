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
	class Writer;

	//
	class TemporaryFile
	{
	public:
		// Creates a temporary file.
		// The file is deleted when the object is destroyed.
		[[nodiscard]] static UniquePtr<TemporaryFile> create();

		virtual ~TemporaryFile() noexcept = default;

		//
		[[nodiscard]] const std::filesystem::path& path() const noexcept { return _path; }

	protected:
		const std::filesystem::path _path;
		explicit TemporaryFile(std::filesystem::path&& path) noexcept
			: _path{ std::move(path) } {}
	};

	// Creates a Blob that references a memory-mapped file.
	// This should be a member function of Blob, but <filesystem> is extremely slow to compile.
	[[nodiscard]] UniquePtr<Blob> createFileBlob(const std::filesystem::path&);
	[[nodiscard]] UniquePtr<Blob> createFileBlob(TemporaryFile&);

	// Creates a Writer thet writes to the specified file.
	// This should be a member function of Writer, but <filesystem> is extremely slow to compile.
	[[nodiscard]] UniquePtr<Writer> createFileWriter(const std::filesystem::path&);
	[[nodiscard]] UniquePtr<Writer> createFileWriter(TemporaryFile&);
}
