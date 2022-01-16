// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/buffer.hpp>
#include <seir_base/unique_ptr.hpp>

#include <cstdint>
#include <cstring>
#include <limits>
#include <string>

namespace seir
{
	//
	class Writer
	{
	public:
		// Creates a Writer that writes to the specified Buffer.
		[[nodiscard]] static UniquePtr<Writer> create(Buffer&, uint64_t* bufferBytes = nullptr);

		// Creates a Writer thet writes to the specified file.
		[[nodiscard]] static UniquePtr<Writer> create(const std::string&);

		virtual ~Writer() noexcept = default;

		//
		virtual bool flush() noexcept = 0;

		// Returns the current offset.
		[[nodiscard]] uint64_t offset() const noexcept { return _offset; }

		//
		bool reserve(uint64_t expectedBytes) noexcept;

		//
		bool seek(uint64_t offset) noexcept;

		//
		[[nodiscard]] uint64_t size() const noexcept { return _size; }

		//
		bool write(const void* data, size_t size) noexcept;

		//
		template <class T>
		bool write(const T& data) noexcept { return write(&data, sizeof data); }

	protected:
		virtual bool reserveImpl(uint64_t capacity) noexcept = 0;
		virtual bool writeImpl(uint64_t offset, const void* data, size_t size) noexcept = 0;

	private:
		uint64_t _size = 0;
		uint64_t _offset = 0;
	};
}

inline seir::UniquePtr<seir::Writer> seir::Writer::create(Buffer& buffer, uint64_t* bufferBytes)
{
	struct BufferWriter final : Writer
	{
		Buffer& _buffer;
		uint64_t* const _bufferBytes;
		// cppcheck-suppress constParameter
		explicit BufferWriter(Buffer& buffer, uint64_t* bufferBytes) noexcept
			: _buffer{ buffer }, _bufferBytes{ bufferBytes } {}
		bool flush() noexcept override { return true; }
		bool reserveImpl(uint64_t capacity) noexcept override
		{
			if constexpr (sizeof(uint64_t) > sizeof(size_t))
				if (capacity > std::numeric_limits<size_t>::max())
					return false;
			return _buffer.tryReserve(static_cast<size_t>(capacity), static_cast<size_t>(_size));
		}
		bool writeImpl(uint64_t offset64, const void* data, size_t size) noexcept override
		{
			if constexpr (sizeof(uint64_t) > sizeof(size_t))
				if (offset64 > std::numeric_limits<size_t>::max())
					return false;
			const auto offset = static_cast<size_t>(offset64);
			if (size > std::numeric_limits<size_t>::max() - offset)
				return false;
			const auto requiredCapacity = offset + size;
			if (requiredCapacity > _buffer.capacity())
			{
				const auto grownCapacity = _buffer.capacity() + _buffer.capacity() / 2;
				if (!_buffer.tryReserve(requiredCapacity > grownCapacity ? requiredCapacity : grownCapacity, static_cast<size_t>(_size)))
					return false;
			}
			std::memcpy(_buffer.data() + offset, data, size);
			if (_bufferBytes && *_bufferBytes < requiredCapacity)
				*_bufferBytes = requiredCapacity;
			return true;
		}
	};
	if (bufferBytes)
		*bufferBytes = 0;
	return makeUnique<Writer, BufferWriter>(buffer, bufferBytes);
}
