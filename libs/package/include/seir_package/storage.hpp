// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/unique_ptr.hpp>

#include <string>

namespace seir
{
	enum class Compression;
	class Inlet;
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

		explicit Storage(UseFileSystem);
		~Storage() noexcept;

		//
		void attach(std::string_view name, SharedPtr<Inlet>&&);

		//
		void attach(std::string_view name, SharedPtr<Inlet>&&, size_t offset, size_t size, Compression, size_t compressedSize);

		//
		bool attachArchive(const SharedPtr<Inlet>&);

		//
		[[nodiscard]] SharedPtr<Inlet> open(const std::string& name) const;

	private:
		const UniquePtr<struct StorageImpl> _impl;
	};
}
