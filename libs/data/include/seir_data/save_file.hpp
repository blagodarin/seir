// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_data/writer.hpp>

namespace seir
{
	//
	class SaveFile : public Writer
	{
	public:
		// Creates a temporary file to replace the specified file.
		[[nodiscard]] static UniquePtr<SaveFile> create(std::string&& path);

		// Replaces the target file with the temporary file contents.
		static bool commit(UniquePtr<SaveFile>&&) noexcept;
	};
}
