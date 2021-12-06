// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_data/storage.hpp>

#include <seir_base/shared_ptr.hpp>
#include <seir_data/blob.hpp>
#include <seir_data/file.hpp>

#include <string>
#include <unordered_map>

namespace seir
{
	struct StorageImpl
	{
		const Storage::UseFileSystem _useFileSystem;
		std::unordered_map<std::string, SharedPtr<Blob>> _attachments;

		StorageImpl(Storage::UseFileSystem useFileSystem) noexcept
			: _useFileSystem{ useFileSystem } {}
	};

	Storage::Storage(UseFileSystem useFileSystem)
		: _impl{ makeUnique<StorageImpl>(useFileSystem) }
	{
	}

	Storage::~Storage() noexcept = default;

	void Storage::attach(std::string_view name, const SharedPtr<Blob>& blob)
	{
		_impl->_attachments.insert_or_assign(std::string{ name }, blob);
	}

	SharedPtr<Blob> Storage::open(const std::string& name) const
	{
		if (_impl->_useFileSystem == UseFileSystem::BeforeAttachments)
			if (auto blob = openFile(name))
				return SharedPtr{ std::move(blob) };
		if (const auto i = _impl->_attachments.find(name); i != _impl->_attachments.end())
			return i->second;
		if (_impl->_useFileSystem == UseFileSystem::AfterAttachments)
			if (auto blob = openFile(name))
				return SharedPtr{ std::move(blob) };
		return {};
	}
}
