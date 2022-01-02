// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/buffer.hpp>
#include <seir_base/unique_ptr.hpp>

#include <cstdint>
#include <limits>

namespace seir
{
	//
	class Writer
	{
	public:
		// Creates a Writer that writes to the specified Buffer.
		template <class T, class A>
		[[nodiscard]] static std::enable_if_t<sizeof(T) == 1, UniquePtr<Writer>> create(Buffer<T, A>&);

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

template <class T, class A>
std::enable_if_t<sizeof(T) == 1, seir::UniquePtr<seir::Writer>> seir::Writer::create(Buffer<T, A>& buffer)
{
	struct BufferWriter final : Writer
	{
		Buffer<T, A>& _buffer;
		// cppcheck-suppress constParameter
		explicit BufferWriter(Buffer<T, A>& buffer) noexcept
			: _buffer{ buffer } {}
		bool flush() noexcept override { return true; }
		bool reserveImpl(uint64_t capacity) noexcept override
		{
			if constexpr (sizeof(uint64_t) > sizeof(size_t))
				if (capacity > std::numeric_limits<size_t>::max())
					return false;
			return _buffer.tryReserve(static_cast<size_t>(capacity));
		}
		bool writeImpl(uint64_t offset64, const void* data, size_t size) noexcept override
		{
			if constexpr (sizeof(uint64_t) > sizeof(size_t))
				if (offset64 > std::numeric_limits<size_t>::max())
					return false;
			const auto offset = static_cast<size_t>(offset64);
			if (size > std::numeric_limits<size_t>::max() - offset
				|| !_buffer.tryReserve(offset + size)) // TODO: Improve performance in reserve-less Writer usage scenarios.
				return false;
			std::memcpy(_buffer.data() + offset, data, size);
			return true;
		}
	};
	return makeUnique<Writer, BufferWriter>(buffer);
}
