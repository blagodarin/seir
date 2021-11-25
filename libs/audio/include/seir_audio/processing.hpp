// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/intrinsics.hpp>

#include <cstddef>
#include <cstdint>

namespace seir
{
	// Minimum alignment for audio data.
	constexpr size_t kAudioAlignment = SEIR_INTRINSICS_SSE ? 16 : 1;

	// Adds 32-bit floats to the output buffer with the same number of interleaved channels.
	void addSamples1D(float* dst, const float* src, size_t length) noexcept;

	// Converts 16-bit integers in [-32768, 32768) to 32-bit floats in [-1, 1)
	// and adds them to the output buffer with the same number of interleaved channels.
	void addSamples1D(float* dst, const int16_t* src, size_t length) noexcept;

	// Adds 32-bit floats to the output buffer with twice the number of interleaved channels.
	void addSamples2x1D(float* dst, const float* src, size_t length) noexcept;

	// Converts 16-bit integers in [-32768, 32768) to 32-bit floats in [-1, 1)
	// and adds them to the output buffer with twice the number of interleaved channels.
	void addSamples2x1D(float* dst, const int16_t* src, size_t length) noexcept;

	// Converts 16-bit integers in [-32768, 32768) to 32-bit floats in [-1, 1)
	// and writes them to the output buffer with the same number of interleaved channels.
	void convertSamples1D(float* dst, const int16_t* src, size_t length) noexcept;

	// Converts 16-bit integers in [-32768, 32768) to 32-bit floats in [-1, 1)
	// and writer them to the output buffer with twice the number of interleaved channels.
	void convertSamples2x1D(float* dst, const int16_t* src, size_t length) noexcept;

	// Duplicates 16-bit values.
	void duplicate1D_16(void* dst, const void* src, size_t length) noexcept;

	// Duplicates 32-bit values.
	void duplicate1D_32(void* dst, const void* src, size_t length) noexcept;
}
