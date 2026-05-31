// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/buffer.hpp>
#include <seir_io/inlet.hpp>

#include <limits>

namespace seir
{
	// An inlet for a Buffer.
	class BufferInlet final : public Inlet
	{
	public:
		//
		constexpr explicit BufferInlet(Buffer&& buffer, size_t maxSize = std::numeric_limits<size_t>::max()) noexcept
			: Inlet{ buffer.data(), maxSize < buffer.capacity() ? maxSize : buffer.capacity() }, _buffer{ std::move(buffer) } {}

	private:
		const Buffer _buffer;
	};
}
