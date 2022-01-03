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
		//
		[[nodiscard]] static UniquePtr<SaveFile> create(std::string&& path);

		//
		static bool commit(UniquePtr<SaveFile>&&) noexcept;
	};
}
