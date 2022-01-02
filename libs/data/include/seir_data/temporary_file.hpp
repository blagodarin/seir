// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

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

	//
	[[nodiscard]] SharedPtr<Blob> createFileBlob(TemporaryFile&);

	//
	[[nodiscard]] UniquePtr<Writer> createFileWriter(TemporaryFile&);
}
