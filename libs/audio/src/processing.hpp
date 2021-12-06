// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstddef>
#include <cstdint>

namespace seir
{
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

	//
	void resampleAdd2x1D(float* dst, size_t dstLength, const float* src, size_t srcOffset, size_t srcStep) noexcept;

	//
	void resampleCopy2x1D(float* dst, size_t dstLength, const float* src, size_t srcOffset, size_t srcStep) noexcept;
}
