// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/shared_ptr.hpp>

#include <string>

namespace seir
{
	class TemporaryFile;

	// Memory-based data source.
	class Blob : public ReferenceCounter
	{
	public:
		// Creates a Blob that references a memory range.
		// NOTE: The range must stay valid for the lifetime of the Blob.
		[[nodiscard]] static SharedPtr<Blob> from(const void* data, size_t size);

		// Creates a Blob that references a part of another Blob.
		[[nodiscard]] static SharedPtr<Blob> from(SharedPtr<Blob>&&, size_t offset, size_t size);

		// Creates a Blob that references a memory-mapped file.
		[[nodiscard]] static SharedPtr<Blob> from(const std::string&);

		// Creates a Blob from a TemporaryFile.
		// NOTE: The TemporaryFile must stay valid for the lifetime of the Blob.
		[[nodiscard]] static SharedPtr<Blob> from(TemporaryFile&);

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
