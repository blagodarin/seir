// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <string_view>

namespace seir
{
	class Blob;
	enum class Compression;
	enum class CompressionLevel;
	template <class>
	class UniquePtr;
	class Writer;

	//
	class Archiver
	{
	public:
		//
		[[nodiscard]] static UniquePtr<Archiver> create(UniquePtr<Writer>&&, Compression);

		virtual ~Archiver() noexcept = default;

		//
		virtual bool add(std::string_view name, const Blob&, CompressionLevel) = 0;

		//
		virtual bool finish() = 0;
	};
}
