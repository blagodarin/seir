// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "archive.hpp"

#include <seir_base/unique_ptr.hpp>

namespace seir
{
	UniquePtr<Archiver> Archiver::create(UniquePtr<Writer>&& writer, Compression compression)
	{
		return createSeirArchiver(std::move(writer), compression);
	}
}
