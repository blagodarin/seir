// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/unique_ptr.hpp>

#include <string>

namespace seir
{
	class Blob;
	enum class Compression;
	template <class>
	class SharedPtr;

	class Storage
	{
	public:
		enum class UseFileSystem
		{
			Never,
			AfterAttachments,
			BeforeAttachments,
		};

		Storage(UseFileSystem);
		~Storage() noexcept;

		//
		void attach(std::string_view name, const SharedPtr<Blob>&);

		//
		void attach(std::string_view name, const SharedPtr<Blob>&, size_t offset, size_t size, Compression, size_t compressedSize);

		//
		[[nodiscard]] SharedPtr<Blob> open(const std::string& name) const;

	private:
		const UniquePtr<struct StorageImpl> _impl;
	};
}
