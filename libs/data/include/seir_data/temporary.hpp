// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_data/writer.hpp>

namespace seir
{
	// Temporary file.
	class TemporaryFile
	{
	public:
		virtual ~TemporaryFile() = default;

		// Returns the path to the temporary file.
		[[nodiscard]] const std::string& path() const noexcept { return _path; }

	protected:
		const std::string _path;
		explicit TemporaryFile(std::string&& path) noexcept
			: _path{ std::move(path) } {}
	};

	// Temporary file writer.
	class TemporaryWriter : public Writer
	{
	public:
		// Creates a temporary file writer.
		// The file may not be visible in the filesystem until it's committed.
		[[nodiscard]] static UniquePtr<TemporaryWriter> create();

		// Commits the temporary file to the filesystem.
		[[nodiscard]] static UniquePtr<TemporaryFile> commit(UniquePtr<TemporaryWriter>&&);
	};
}
