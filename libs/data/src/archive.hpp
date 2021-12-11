// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_data/archive.hpp>

namespace seir
{
	UniquePtr<Archiver> createSeirArchiver(UniquePtr<Writer>&&, Compression);
}
