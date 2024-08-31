// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_data/blob.hpp>

#include <span>
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

		//
		[[nodiscard]] constexpr const void* peek(size_t bytes) const noexcept;

		// If there are at least sizeof(T) bytes from the current offset to the end of the blob,
		// returns the pointer to the data at the current offset and advances the offset.
		// Otherwise, returns nullptr and doesn't change the offset.
		template <class T>
		[[nodiscard]] constexpr const T* read() noexcept;

		//
		template <class T>
		[[nodiscard]] constexpr std::span<const T> readArray(size_t maxElements) noexcept;

		//
		[[nodiscard]] constexpr std::pair<const void*, size_t> readBlocks(size_t maxBlocks, size_t blockSize) noexcept;

		// Retrieves the next line of text (including newline),
		// and advances the current offset accordingly.
		// Returns empty string if there is no more data to read.
		[[nodiscard]] constexpr std::string_view readLine() noexcept;

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

constexpr const void* seir::Reader::peek(size_t bytes) const noexcept
{
	return _blob.size() - _offset >= bytes
		? static_cast<const std::byte*>(_blob.data()) + _offset
		: nullptr;
}

template <class T>
[[nodiscard]] constexpr const T* seir::Reader::read() noexcept
{
	if (_blob.size() - _offset < sizeof(T))
		return nullptr;
	const auto result = reinterpret_cast<const T*>(static_cast<const std::byte*>(_blob.data()) + _offset);
	_offset += sizeof(T);
	return result;
}

template <class T>
[[nodiscard]] constexpr std::span<const T> seir::Reader::readArray(size_t maxElements) noexcept
{
	const auto data = reinterpret_cast<const T*>(static_cast<const std::byte*>(_blob.data()) + _offset);
	auto count = (_blob.size() - _offset) / sizeof(T);
	if (count > maxElements)
		count = maxElements;
	_offset += count * sizeof(T);
	return { data, count };
}

[[nodiscard]] constexpr std::pair<const void*, size_t> seir::Reader::readBlocks(size_t maxBlocks, size_t blockSize) noexcept
{
	const auto data = static_cast<const std::byte*>(_blob.data()) + _offset;
	auto count = (_blob.size() - _offset) / blockSize;
	if (count > maxBlocks)
		count = maxBlocks;
	_offset += count * blockSize;
	return { data, count };
}

constexpr std::string_view seir::Reader::readLine() noexcept
{
	const auto data = static_cast<const char*>(_blob.data()) + _offset;
	const auto maxLength = _blob.size() - _offset;
	size_t length = 0;
	while (length < maxLength)
		if (const auto next = data[length++]; next == '\r')
		{
			if (length < maxLength && data[length] == '\n')
				++length;
			break;
		}
		else if (next == '\n')
			break;
	_offset += length;
	return { data, length };
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
	if (bytes > _blob.size() - _offset)
		return false;
	_offset += bytes;
	return true;
}
