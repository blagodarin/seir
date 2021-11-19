// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_data/blob.hpp>

namespace seir
{
	class Reader
	{
	public:
		constexpr explicit Reader(const Blob& blob) noexcept
			: _blob{ blob } {}

		// Returns the current offset.
		[[nodiscard]] constexpr size_t offset() const noexcept { return _offset; }

		// If there are at least sizeof(T) bytes from the current offset to the end of the blob,
		// returns the pointer to the data at the current offset and advances the offset.
		// Otherwise, returns nullptr and doesn't change the offset.
		template <typename T>
		[[nodiscard]] constexpr const T* read() noexcept;

		// Sets the current offset to the specified value.
		constexpr bool seek(size_t offset) noexcept;

		// Returns the size of the underlying Blob.
		[[nodiscard]] constexpr size_t size() const noexcept { return _blob.size(); }

		// Advances the current offset by the specified number of bytes.
		constexpr bool skip(size_t bytes) noexcept;

	private:
		const Blob& _blob;
		size_t _offset = 0;
	};
}

template <typename T>
[[nodiscard]] constexpr const T* seir::Reader::read() noexcept
{
	assert(_offset <= _blob.size());
	if (_blob.size() - _offset < sizeof(T))
		return nullptr;
	const auto result = &_blob.get<T>(_offset);
	_offset += sizeof(T);
	return result;
}

constexpr bool seir::Reader::seek(size_t offset) noexcept
{
	if (offset > _blob.size())
		return false;
	_offset = offset;
	return true;
}

constexpr bool seir::Reader::skip(size_t bytes) noexcept
{
	assert(_offset <= _blob.size());
	if (bytes > _blob.size() - _offset)
		return false;
	_offset += bytes;
	return true;
}
