// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_package/archive.hpp>

#include <seir_base/endian.hpp>

namespace seir
{
	template <class>
	class SharedPtr;
	class Storage;

	constexpr uint32_t kSeirFileID = seir::makeCC('\xDF', 'S', 'a', '\x01');
	bool attachSeirArchive(Storage&, const SharedPtr<Blob>&);
	UniquePtr<Archiver> createSeirArchiver(UniquePtr<Writer>&&, Compression);
}
