// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/shared_ptr.hpp>

#include <cassert>
#include <cstddef>

namespace seir
{
	// Memory-based data source.
	class Blob : public ReferenceCounter
	{
	public:
		// Creates a Blob that references a memory range.
		// The range must stay valid for the lifetime of the Blob.
		[[nodiscard]] static UniquePtr<Blob> from(const void* data, size_t size);

		// Creates a Blob that references a part of another Blob.
		[[nodiscard]] static UniquePtr<Blob> from(const SharedPtr<Blob>&, size_t offset, size_t size);

		// Returns the data pointer.
		[[nodiscard]] constexpr const void* data() const noexcept { return _data; }

		//
		template <typename T>
		[[nodiscard]] constexpr const T& get(size_t offset) const noexcept;

		// Returns the size of the data.
		[[nodiscard]] constexpr size_t size() const noexcept { return _size; }

	protected:
		const void* const _data;
		const size_t _size;
		constexpr Blob(const void* data, size_t size) noexcept
			: _data{ data }, _size{ size } {}
	};
}

inline seir::UniquePtr<seir::Blob> seir::Blob::from(const void* data, size_t size)
{
	struct Wrapper final : Blob
	{
		constexpr Wrapper(const void* data, size_t size) noexcept
			: Blob{ data, size } {}
	};
	return makeUnique<Blob, Wrapper>(data, size);
}

inline seir::UniquePtr<seir::Blob> seir::Blob::from(const SharedPtr<Blob>& parent, size_t offset, size_t size)
{
	class Subrange : public Blob
	{
	public:
		Subrange(const SharedPtr<Blob>& parent, size_t offset, size_t size)
			: Blob{ static_cast<const std::byte*>(parent->data()) + offset, size }, _parent{ parent } {}

	private:
		const SharedPtr<Blob> _parent;
	};
	if (offset > parent->size())
		offset = parent->size();
	const auto maxSize = parent->size() - offset;
	return makeUnique<Blob, Subrange>(parent, offset, size < maxSize ? size : maxSize);
}

template <typename T>
[[nodiscard]] constexpr const T& seir::Blob::get(size_t offset) const noexcept
{
	assert(offset <= _size && sizeof(T) <= _size - offset);
	return *reinterpret_cast<const T*>(static_cast<const std::byte*>(_data) + offset);
}
