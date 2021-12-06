// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_data/storage.hpp>

#include <seir_base/shared_ptr.hpp>
#include <seir_data/blob.hpp>

#include <string>
#include <unordered_map>

namespace seir
{
	struct StorageImpl
	{
		std::unordered_map<std::string, SharedPtr<Blob>> _attachments;
	};

	Storage::Storage()
		: _impl{ makeUnique<StorageImpl>() }
	{
	}

	Storage::~Storage() noexcept = default;

	void Storage::attach(std::string_view name, const SharedPtr<Blob>& blob)
	{
		_impl->_attachments.insert_or_assign(std::string{ name }, blob);
	}

	SharedPtr<Blob> Storage::open(const std::string& name) const
	{
		if (const auto i = _impl->_attachments.find(name); i != _impl->_attachments.end())
			return i->second;
		return {};
	}
}
