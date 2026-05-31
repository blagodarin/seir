// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_package/storage.hpp>

#include <seir_compression/compression.hpp>
#include <seir_io/buffer_inlet.hpp>
#include "archive.hpp"

#include <cassert>
#include <unordered_map>

namespace
{
	struct Attachment
	{
		seir::SharedPtr<seir::Inlet> _inlet;
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

	void Storage::attach(std::string_view name, SharedPtr<Inlet>&& inlet)
	{
		const auto size = inlet->size();
		_impl->_attachments.insert_or_assign(std::string{ name }, Attachment{ std::move(inlet), 0, size, size, Compression::None });
	}

	void Storage::attach(std::string_view name, SharedPtr<Inlet>&& inlet, size_t offset, size_t size, Compression compression, size_t compressedSize)
	{
		assert(offset <= inlet->size() && compressedSize <= inlet->size() - offset);
		_impl->_attachments.insert_or_assign(std::string{ name }, Attachment{ std::move(inlet), offset, size, compressedSize, compression });
	}

	bool Storage::attachArchive(const SharedPtr<Inlet>& inlet)
	{
		if (inlet)
			if (const auto id = inlet->get<uint32_t>(0))
				switch (*id)
				{
				case kSeirFileID:
					return attachSeirArchive(*this, inlet);
				default:
					break;
				}
		return {};
	}

	SharedPtr<Inlet> Storage::open(const std::string& name) const
	{
		if (_impl->_useFileSystem == UseFileSystem::BeforeAttachments)
			if (auto inlet = fromFile(name))
				return inlet;
		if (const auto i = _impl->_attachments.find(name); i != _impl->_attachments.end())
		{
			if (i->second._compression == Compression::None)
			{
				return i->second._offset == 0 && i->second._uncompressedSize == i->second._inlet->size()
					? i->second._inlet
					: Inlet::from(SharedPtr{ i->second._inlet }, i->second._offset, i->second._uncompressedSize);
			}
			if (const auto decompressor = Decompressor::create(i->second._compression))
			{
				Buffer buffer{ i->second._uncompressedSize };
				if (decompressor->decompress(buffer.data(), i->second._uncompressedSize, static_cast<const std::byte*>(i->second._inlet->data()) + i->second._offset, i->second._compressedSize))
					return makeShared<Inlet, BufferInlet>(std::move(buffer), i->second._uncompressedSize);
			}
			return {};
		}
		if (_impl->_useFileSystem == UseFileSystem::AfterAttachments)
			if (auto inlet = fromFile(name))
				return inlet;
		return {};
	} // NOLINT(clang-analyzer-cplusplus.NewDeleteLeaks)
}
