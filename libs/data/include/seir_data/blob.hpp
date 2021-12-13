// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/buffer.hpp>
#include <seir_base/shared_ptr.hpp>

#include <cassert>
#include <cstddef>
#include <limits>

namespace seir
{
	// Memory-based data source.
	class Blob : public ReferenceCounter
	{
	public:
		// Creates a Blob that references a memory range.
		// NOTE: The range must stay valid for the lifetime of the Blob.
		[[nodiscard]] static SharedPtr<Blob> from(const void* data, size_t size);

		// Creates a Blob from a Buffer.
		template <class T, class A>
		[[nodiscard]] static SharedPtr<Blob> from(Buffer<T, A>&&, size_t maxSize = std::numeric_limits<size_t>::max());

		// Creates a Blob that references a part of another Blob.
		[[nodiscard]] static SharedPtr<Blob> from(SharedPtr<Blob>&&, size_t offset, size_t size);

		// Returns the data pointer.
		[[nodiscard]] constexpr const void* data() const noexcept { return _data; }

		//
		template <typename T>
		[[nodiscard]] constexpr const T* get(size_t offset, size_t count = 1) const noexcept;

		// Returns the size of the data.
		[[nodiscard]] constexpr size_t size() const noexcept { return _size; }

	protected:
		const void* const _data;
		const size_t _size;
		constexpr Blob(const void* data, size_t size) noexcept
			: _data{ data }, _size{ size } {}
	};
}

inline seir::SharedPtr<seir::Blob> seir::Blob::from(const void* data, size_t size)
{
	struct ReferenceBlob final : Blob
	{
		constexpr ReferenceBlob(const void* data, size_t size) noexcept
			: Blob{ data, size } {}
	};
	return makeShared<Blob, ReferenceBlob>(data, size);
}

template <class T, class A>
seir::SharedPtr<seir::Blob> seir::Blob::from(Buffer<T, A>&& buffer, size_t maxSize)
{
	struct BufferBlob final : Blob
	{
		const Buffer<T, A> _buffer;
		constexpr BufferBlob(Buffer<T, A>&& buffer, size_t size) noexcept
			: Blob{ buffer.data(), size }, _buffer{ std::move(buffer) } {}
	};
	const auto maxMaxSize = buffer.capacity() * sizeof(T);
	return makeShared<Blob, BufferBlob>(std::move(buffer), maxSize < maxMaxSize ? maxSize : maxMaxSize);
}

inline seir::SharedPtr<seir::Blob> seir::Blob::from(SharedPtr<Blob>&& parent, size_t offset, size_t size)
{
	struct SubBlob final : Blob
	{
		const SharedPtr<Blob> _parent;
		constexpr SubBlob(SharedPtr<Blob>&& parent, size_t offset, size_t size) noexcept
			: Blob{ static_cast<const std::byte*>(parent->data()) + offset, size }, _parent{ std::move(parent) } {}
	};
	if (offset > parent->size())
		offset = parent->size();
	const auto maxSize = parent->size() - offset;
	return makeShared<Blob, SubBlob>(std::move(parent), offset, size < maxSize ? size : maxSize);
}

template <typename T>
constexpr const T* seir::Blob::get(size_t offset, size_t count) const noexcept
{
	return offset <= _size && count <= (_size - offset) / sizeof(T) // Multiplication is faster, but may overflow.
		? reinterpret_cast<const T*>(static_cast<const std::byte*>(_data) + offset)
		: nullptr;
}
