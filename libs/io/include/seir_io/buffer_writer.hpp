// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_io/writer.hpp>

namespace seir
{
	class Buffer;

	// A Writer that writes to a Buffer.
	class BufferWriter final : public Writer
	{
	public:
		//
		constexpr explicit BufferWriter(Buffer&, uint64_t* bufferBytes = nullptr) noexcept;

		bool flush() noexcept override;

	private:
		bool reserveImpl(uint64_t capacity) noexcept override;
		bool writeImpl(uint64_t offset64, const void* data, size_t size) noexcept override;

	private:
		Buffer& _buffer;
		uint64_t* const _bufferBytes;
	};
}

constexpr seir::BufferWriter::BufferWriter(Buffer& buffer, uint64_t* bufferBytes) noexcept
	: _buffer{ buffer }
	, _bufferBytes{ bufferBytes }
{
	if (bufferBytes)
		*bufferBytes = 0;
}
