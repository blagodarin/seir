// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/buffer.hpp>
#include <seir_data/blob.hpp>

#include <limits>

namespace seir
{
	// A Blob backed by a Buffer.
	class BufferBlob final : public Blob
	{
	public:
		//
		constexpr explicit BufferBlob(Buffer&& buffer, size_t maxSize = std::numeric_limits<size_t>::max()) noexcept
			: Blob{ buffer.data(), maxSize < buffer.capacity() ? maxSize : buffer.capacity() }, _buffer{ std::move(buffer) } {}

	private:
		const Buffer _buffer;
	};
}
