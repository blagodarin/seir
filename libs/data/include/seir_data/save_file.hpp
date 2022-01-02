// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_data/writer.hpp>

#include <string>

namespace seir
{
	//
	class SaveFile : public Writer
	{
	public:
		//
		[[nodiscard]] static UniquePtr<SaveFile> create(std::string&& path);

		//
		virtual bool commit() = 0;
	};
}
