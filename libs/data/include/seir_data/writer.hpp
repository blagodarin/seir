// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/unique_ptr.hpp>

#include <cstdint>
#include <string>

namespace seir
{
	//
	class Writer
	{
	public:
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
