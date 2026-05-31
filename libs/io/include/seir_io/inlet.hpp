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
	class Inlet : public ReferenceCounter
	{
	public:
		// Creates an inlet from a memory range.
		// NOTE: The range must stay valid for the lifetime of the inlet.
		[[nodiscard]] static SharedPtr<Inlet> from(const void* data, size_t size);

		// Creates an inlet from a part of another Inlet.
		[[nodiscard]] static SharedPtr<Inlet> from(SharedPtr<Inlet>&&, size_t offset, size_t size);

		// Creates an inlet from a memory-mapped file.
		[[nodiscard]] static SharedPtr<Inlet> from(const std::string&);

		// Creates an inlet from a TemporaryFile.
		// NOTE: The TemporaryFile must stay valid for the lifetime of the inlet.
		[[nodiscard]] static SharedPtr<Inlet> from(TemporaryFile&);

		virtual ~Inlet() noexcept = default;

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
		constexpr Inlet(const void* data, size_t size) noexcept
			: _data{ data }, _size{ size } {}
	};

	// Convenience short-hand for loading from files.
	[[nodiscard]] inline SharedPtr<Inlet> fromFile(const std::string& path)
	{
		return Inlet::from(path);
	}
}

inline seir::SharedPtr<seir::Inlet> seir::Inlet::from(const void* data, size_t size)
{
	struct MemoryInlet final : Inlet
	{
		constexpr MemoryInlet(const void* data, size_t size) noexcept
			: Inlet{ data, size } {}
	};
	return makeShared<Inlet, MemoryInlet>(data, size);
}

inline seir::SharedPtr<seir::Inlet> seir::Inlet::from(SharedPtr<Inlet>&& parent, size_t offset, size_t size)
{
	struct SubInlet final : Inlet
	{
		const SharedPtr<Inlet> _parent;
		constexpr SubInlet(SharedPtr<Inlet>&& parent, size_t offset, size_t size) noexcept
			: Inlet{ static_cast<const std::byte*>(parent->data()) + offset, size }, _parent{ std::move(parent) } {}
	};
	if (offset > parent->size())
		offset = parent->size();
	const auto maxSize = parent->size() - offset;
	return makeShared<Inlet, SubInlet>(std::move(parent), offset, size < maxSize ? size : maxSize);
}

template <typename T>
constexpr const T* seir::Inlet::get(size_t offset, size_t count) const noexcept
{
	return offset <= _size && count <= (_size - offset) / sizeof(T) // Multiplication is faster, but may overflow.
		? reinterpret_cast<const T*>(static_cast<const std::byte*>(_data) + offset)
		: nullptr;
}
