// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "archive.hpp"

#include <seir_base/endian.hpp>
#include <seir_base/unique_ptr.hpp>
#include <seir_data/blob.hpp>
#include <seir_data/compression.hpp>
#include <seir_data/writer.hpp>

#include <array>
#include <string>
#include <vector>

namespace
{
	constexpr uint32_t kSeirFileID = seir::makeCC('\xDF', 'S', 'a', '\x01');
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
	};

	static_assert(sizeof(SeirBlockInfo) == 16);

	struct SeirFileHeader
	{
		uint32_t _id = kSeirFileID;
		SeirCompression _compression = SeirCompression::None;
		uint8_t _reserved[7]{};
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
				seir::Buffer<std::byte> metaBuffer;
				if (!metaBuffer.tryReserve(metaSize))
					return false;
				{
					const auto metaWriter = seir::Writer::to(metaBuffer);
					for (const auto& file : _files)
						metaWriter->write(file._blockInfo);
					for (const auto& file : _files)
					{
						metaWriter->write(static_cast<uint8_t>(file._name.size()));
						metaWriter->write(file._name.data(), file._name.size());
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
					if (!_compressionBuffer.tryReserve((maxCompressedSize + (MiB - 1)) & ~(MiB - 1), false))
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
		seir::Buffer<std::byte> _compressionBuffer;
		SeirFileHeader _header;
		std::vector<FileInfo> _files;
		uint64_t _lastOffset = 0;
	};
}

namespace seir
{
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
