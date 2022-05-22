// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "archive.hpp"

#include <seir_base/buffer.hpp>
#include <seir_data/blob.hpp>
#include <seir_data/buffer_writer.hpp>
#include <seir_data/compression.hpp>
#include <seir_data/storage.hpp>
#include <seir_data/writer.hpp>

#include <array>
#include <limits>
#include <string>
#include <vector>

namespace
{
	constexpr size_t kSeirBlockAlignmentBits = 4;
	constexpr size_t kSeirBlockAlignment = 1 << kSeirBlockAlignmentBits;

	enum class SeirCompression : uint8_t
	{
		None = 0,
		Zlib = 1,
		Zstd = 2,
	};

	struct SeirBlockInfo
	{
		uint32_t _alignedOffset = 0;
		uint32_t _archivedSize = 0;
		uint32_t _originalSize = 0;
		uint32_t _flags = 0;

		[[nodiscard]] constexpr uint64_t offset() const noexcept { return uint64_t{ _alignedOffset } << kSeirBlockAlignmentBits; }
	};

	static_assert(sizeof(SeirBlockInfo) == 16);

	struct SeirFileHeader
	{
		uint32_t _id = seir::kSeirFileID;
		SeirCompression _compression = SeirCompression::None;
		uint8_t _reserved8 = 0;
		uint16_t _reserved16 = 0;
		uint32_t _reserved32 = 0;
		uint32_t _fileCount = 0;
		SeirBlockInfo _metaBlock;
	};

	static_assert(sizeof(SeirFileHeader) == 32);

	class SeirArchiver final : public seir::Archiver
	{
	public:
		SeirArchiver(seir::UniquePtr<seir::Writer>&& writer, seir::UniquePtr<seir::Compressor>&& compressor, seir::Compression compression) noexcept
			: _writer{ std::move(writer) }
			, _compressor{ std::move(compressor) }
		{
			switch (compression)
			{
			case seir::Compression::None: break;
			case seir::Compression::Zlib: _header._compression = SeirCompression::Zlib; break;
			case seir::Compression::Zstd: _header._compression = SeirCompression::Zstd; break;
			}
		}

		bool add(std::string_view name, const seir::Blob& blob, seir::CompressionLevel compressionLevel) override
		{
			if (name.size() > std::numeric_limits<uint8_t>::max())
				return false;
			if constexpr (sizeof(size_t) > sizeof(uint32_t))
				if (_files.size() == std::numeric_limits<uint32_t>::max())
					return false;
			SeirBlockInfo blockInfo;
			if (!writeBlock(blockInfo, blob.data(), blob.size(), compressionLevel))
				return false;
			_files.emplace_back(name, blockInfo);
			return true;
		}

		bool finish() override
		{
			_header._fileCount = static_cast<uint32_t>(_files.size());
			if (!_files.empty())
			{
				size_t metaSize = _files.size() * sizeof(SeirBlockInfo);
				for (const auto& file : _files)
					metaSize += 1 + file._name.size();
				seir::Buffer metaBuffer;
				if (!metaBuffer.tryReserve(metaSize, 0))
					return false;
				{
					seir::BufferWriter metaWriter{ metaBuffer };
					for (const auto& file : _files)
						metaWriter.write(file._blockInfo);
					for (const auto& file : _files)
					{
						metaWriter.write(static_cast<uint8_t>(file._name.size()));
						metaWriter.write(file._name.data(), file._name.size());
					}
				}
				if (!writeBlock(_header._metaBlock, metaBuffer.data(), metaSize, seir::CompressionLevel::BestCompression))
					return false;
			}
			else
			{
				_header._metaBlock = {};
				_lastOffset = sizeof _header;
			}
			return _writer->seek(0) && _writer->write(_header);
		}

	private:
		struct FileInfo
		{
			std::string _name;
			SeirBlockInfo _blockInfo;
			FileInfo(std::string_view name, const SeirBlockInfo& blockInfo)
				: _name{ name }, _blockInfo{ blockInfo } {}
		};

		bool writeBlock(SeirBlockInfo& blockInfo, const void* data, size_t size, seir::CompressionLevel compressionLevel)
		{
			const auto requiredPadding = (~_lastOffset + 1) & (kSeirBlockAlignment - 1);
			const auto alignedOffset = (_lastOffset + requiredPadding) >> kSeirBlockAlignmentBits;
			if (alignedOffset > std::numeric_limits<uint32_t>::max())
				return false;
			const auto originalSize = static_cast<uint32_t>(size);
			if constexpr (sizeof(size_t) > sizeof(uint32_t))
				if (originalSize != size)
					return false;
			auto dataToWrite = data;
			auto archivedSize = originalSize;
			if (_compressor)
			{
				if (!_compressor->prepare(compressionLevel))
					return false;
				const auto maxCompressedSize = _compressor->maxCompressedSize(originalSize);
				if (_compressionBuffer.capacity() < maxCompressedSize)
				{
					constexpr auto MiB = size_t{ 1 } << 20;
					if (!_compressionBuffer.tryReserve((maxCompressedSize + (MiB - 1)) & ~(MiB - 1), 0))
						return false;
				}
				const auto compressedSize = _compressor->compress(_compressionBuffer.data(), _compressionBuffer.capacity(), dataToWrite, originalSize);
				if (!compressedSize)
					return false;
				if (compressedSize < originalSize)
				{
					dataToWrite = _compressionBuffer.data();
					archivedSize = static_cast<uint32_t>(compressedSize);
				}
			}
			if (!_writer->seek(_lastOffset))
				return false;
			if (requiredPadding > 0)
			{
				const std::array<std::byte, kSeirBlockAlignment - 1> padding{};
				if (!_writer->write(padding.data(), static_cast<size_t>(requiredPadding)))
					return false;
			}
			if (!_writer->write(dataToWrite, archivedSize))
				return false;
			blockInfo._alignedOffset = static_cast<uint32_t>(alignedOffset);
			blockInfo._archivedSize = archivedSize;
			blockInfo._originalSize = originalSize;
			_lastOffset = _writer->offset();
			return true;
		}

	private:
		const seir::UniquePtr<seir::Writer> _writer;
		const seir::UniquePtr<seir::Compressor> _compressor;
		seir::Buffer _compressionBuffer;
		SeirFileHeader _header;
		std::vector<FileInfo> _files;
		uint64_t _lastOffset = 0;
	};
}

namespace seir
{
	bool attachSeirArchive(Storage& storage, SharedPtr<Blob>&& blob)
	{
		const auto fileHeader = blob->get<SeirFileHeader>(0);
		if (!fileHeader
			|| fileHeader->_id != seir::kSeirFileID
			|| fileHeader->_reserved8 || fileHeader->_reserved16 || fileHeader->_reserved32)
			return false;
		if (!fileHeader->_fileCount)
			return true;
		if (fileHeader->_metaBlock._archivedSize > fileHeader->_metaBlock._originalSize
			|| fileHeader->_metaBlock._flags)
			return false;
		auto metaBlock = blob->get<std::byte>(static_cast<size_t>(fileHeader->_metaBlock.offset()), fileHeader->_metaBlock._archivedSize);
		if (!metaBlock)
			return false;
		auto compression = Compression::None; // Invalid compression also maps to None, which may be OK if nothing is actually compressed and correctly fails otherwise.
		switch (fileHeader->_compression)
		{
		case SeirCompression::None: break;
		case SeirCompression::Zlib: compression = Compression::Zlib; break;
		case SeirCompression::Zstd: compression = Compression::Zstd; break;
		}
		seir::Buffer metaBuffer;
		if (fileHeader->_metaBlock._archivedSize < fileHeader->_metaBlock._originalSize)
		{
			const auto decompressor = seir::Decompressor::create(compression);
			if (!decompressor
				|| !metaBuffer.tryReserve(fileHeader->_metaBlock._originalSize, 0)
				|| !decompressor->decompress(metaBuffer.data(), fileHeader->_metaBlock._originalSize, metaBlock, fileHeader->_metaBlock._archivedSize))
				return false;
			metaBlock = metaBuffer.data();
		}
		if (fileHeader->_fileCount > fileHeader->_metaBlock._originalSize / sizeof(SeirBlockInfo))
			return false;
		auto nameOffset = fileHeader->_fileCount * sizeof(SeirBlockInfo);
		for (auto i = reinterpret_cast<const SeirBlockInfo*>(metaBlock), end = i + fileHeader->_fileCount; i != end; ++i)
		{
			if (nameOffset == fileHeader->_metaBlock._originalSize)
				break;
			const auto nameSize = std::to_integer<uint8_t>(metaBlock[nameOffset++]);
			if (nameOffset + nameSize > fileHeader->_metaBlock._originalSize)
				break;
			storage.attach(std::string{ reinterpret_cast<const char*>(metaBlock + nameOffset), nameSize }, SharedPtr{ blob }, static_cast<size_t>(i->offset()), i->_originalSize, compression, i->_archivedSize);
			nameOffset += nameSize;
		}
		return true;
	}

	UniquePtr<Archiver> createSeirArchiver(UniquePtr<Writer>&& writer, Compression compression)
	{
		UniquePtr<seir::Compressor> compressor;
		if (compression != Compression::None)
		{
			compressor = Compressor::create(compression);
			if (!compressor)
				return {};
		}
		auto archiver = makeUnique<Archiver, SeirArchiver>(std::move(writer), std::move(compressor), compression);
		if (!archiver->finish())
			return {};
		return archiver;
	}
}
