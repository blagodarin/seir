// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_data/blob.hpp>

#include <string_view>

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

		// Retrieves the next line of text (excluding newline),
		// and advances the current offset accordingly (including newline).
		// Returns false if there is no more data to read.
		[[nodiscard]] constexpr bool readLine(std::string_view&) noexcept;

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

constexpr bool seir::Reader::readLine(std::string_view& line) noexcept
{
	const auto data = static_cast<const char*>(_blob.data()) + _offset;
	const auto size = _blob.size() - _offset;
	for (size_t i = 0;; ++i)
	{
		if (i == size)
		{
			line = { data, i };
			_offset += i;
			return size > 0;
		}
		if (data[i] == '\n')
		{
			line = { data, i++ };
			_offset += i;
			return true;
		}
		if (data[i] == '\r')
		{
			line = { data, i++ };
			_offset += i + static_cast<bool>(i != size && data[i] == '\n');
			return true;
		}
	}
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
