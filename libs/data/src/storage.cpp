// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_data/storage.hpp>

#include <seir_data/blob.hpp>
#include <seir_data/compression.hpp>
#include <seir_data/file.hpp>

#include <string>
#include <unordered_map>

namespace
{
	struct Attachment
	{
		seir::SharedPtr<seir::Blob> _blob;
		size_t _offset = 0;
		size_t _uncompressedSize = 0;
		size_t _compressedSize = 0;
		seir::Compression _compression = seir::Compression::None;
	};
}

namespace seir
{
	struct StorageImpl
	{
		const Storage::UseFileSystem _useFileSystem;
		std::unordered_map<std::string, Attachment> _attachments;

		explicit StorageImpl(Storage::UseFileSystem useFileSystem) noexcept
			: _useFileSystem{ useFileSystem } {}
	};

	Storage::Storage(UseFileSystem useFileSystem)
		: _impl{ makeUnique<StorageImpl>(useFileSystem) }
	{
	}

	Storage::~Storage() noexcept = default;

	void Storage::attach(std::string_view name, const SharedPtr<Blob>& blob)
	{
		_impl->_attachments.insert_or_assign(std::string{ name }, Attachment{ blob, 0, blob->size(), blob->size(), Compression::None });
	}

	void Storage::attach(std::string_view name, const SharedPtr<Blob>& blob, size_t offset, size_t size, Compression compression, size_t compressedSize)
	{
		assert(compression != Compression::None);
		assert(offset <= blob->size() && compressedSize <= blob->size() - offset);
		_impl->_attachments.insert_or_assign(std::string{ name }, Attachment{ blob, offset, size, compressedSize, compression });
	}

	SharedPtr<Blob> Storage::open(const std::string& name) const
	{
		if (_impl->_useFileSystem == UseFileSystem::BeforeAttachments)
			if (auto blob = createFileBlob(name))
				return SharedPtr{ std::move(blob) };
		if (const auto i = _impl->_attachments.find(name); i != _impl->_attachments.end())
		{
			if (i->second._compression == Compression::None)
				return i->second._blob;
			if (const auto decompressor = Decompressor::create(i->second._compression))
			{
				Buffer<std::byte> buffer{ i->second._uncompressedSize };
				if (decompressor->decompress(buffer.data(), i->second._uncompressedSize, static_cast<const std::byte*>(i->second._blob->data()) + i->second._offset, i->second._compressedSize))
					return SharedPtr{ Blob::from(std::move(buffer), i->second._uncompressedSize) };
			}
			return {};
		}
		if (_impl->_useFileSystem == UseFileSystem::AfterAttachments)
			if (auto blob = createFileBlob(name))
				return SharedPtr{ std::move(blob) };
		return {};
	}
}
