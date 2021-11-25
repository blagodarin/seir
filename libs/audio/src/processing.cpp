// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_audio/processing.hpp>

namespace seir
{
	void addSamples1D(float* dst, const float* src, size_t length) noexcept
	{
		// No manual SSE optimization succeeded.
		for (size_t i = 0; i < length; ++i)
			dst[i] += src[i];
	}

	void addSamples1D(float* dst, const int16_t* src, size_t length) noexcept
	{
		constexpr auto unit = 1.f / 32768.f;
#if SEIR_INTRINSICS_SSE // 10-20% faster with MSVC.
		for (; length >= 8; length -= 8)
		{
			const auto input = _mm_load_si128(reinterpret_cast<const __m128i*>(src));
			src += 8;
			_mm_store_ps(dst, _mm_add_ps(_mm_load_ps(dst), _mm_mul_ps(_mm_set1_ps(unit), _mm_cvtepi32_ps(_mm_cvtepi16_epi32(input)))));
			dst += 4;
			_mm_store_ps(dst, _mm_add_ps(_mm_load_ps(dst), _mm_mul_ps(_mm_set1_ps(unit), _mm_cvtepi32_ps(_mm_cvtepi16_epi32(_mm_srli_si128(input, 8))))));
			dst += 4;
		}
		for (; length > 0; --length) // For some reason it's faster than i-based loop with the preceding SSE-optimized loop, but slower without one.
			*dst++ += static_cast<float>(*src++) * unit;
#else
		for (size_t i = 0; i < length; ++i)
			dst[i] += static_cast<float>(src[i]) * unit;
#endif
	}

	void addSamples2x1D(float* dst, const float* src, size_t length) noexcept
	{
#if SEIR_INTRINSICS_SSE // 150-200% faster with MSVC.
		for (; length >= 4; length -= 4)
		{
			const auto input = _mm_load_ps(src);
			src += 4;
			_mm_store_ps(dst, _mm_add_ps(_mm_load_ps(dst), _mm_unpacklo_ps(input, input)));
			dst += 4;
			_mm_store_ps(dst, _mm_add_ps(_mm_load_ps(dst), _mm_unpackhi_ps(input, input)));
			dst += 4;
		}
#endif
		for (; length > 0; --length)
		{
			const auto value = *src++;
			*dst++ += value;
			*dst++ += value;
		}
	}

	void addSamples2x1D(float* dst, const int16_t* src, size_t length) noexcept
	{
		constexpr auto unit = 1.f / 32768.f;
#if SEIR_INTRINSICS_SSE // 150-170% faster with MSVC.
		for (; length >= 8; length -= 8)
		{
			const auto input = _mm_load_si128(reinterpret_cast<const __m128i*>(src));
			src += 8;
			const auto normalized1 = _mm_mul_ps(_mm_set1_ps(unit), _mm_cvtepi32_ps(_mm_cvtepi16_epi32(input)));
			const auto normalized2 = _mm_mul_ps(_mm_set1_ps(unit), _mm_cvtepi32_ps(_mm_cvtepi16_epi32(_mm_srli_si128(input, 8))));
			_mm_store_ps(dst, _mm_add_ps(_mm_load_ps(dst), _mm_unpacklo_ps(normalized1, normalized1)));
			dst += 4;
			_mm_store_ps(dst, _mm_add_ps(_mm_load_ps(dst), _mm_unpackhi_ps(normalized1, normalized1)));
			dst += 4;
			_mm_store_ps(dst, _mm_add_ps(_mm_load_ps(dst), _mm_unpacklo_ps(normalized2, normalized2)));
			dst += 4;
			_mm_store_ps(dst, _mm_add_ps(_mm_load_ps(dst), _mm_unpackhi_ps(normalized2, normalized2)));
			dst += 4;
		}
#endif
		for (; length > 0; --length)
		{
			const auto value = static_cast<float>(*src++) * unit;
			*dst++ += value;
			*dst++ += value;
		}
	}

	void convertSamples1D(float* dst, const int16_t* src, size_t length) noexcept
	{
		constexpr auto unit = 1.f / 32768.f;
#if SEIR_INTRINSICS_SSE // 1-5% faster with MSVC.
		for (; length >= 8; length -= 8)
		{
			const auto input = _mm_load_si128(reinterpret_cast<const __m128i*>(src));
			src += 8;
			_mm_store_ps(dst, _mm_mul_ps(_mm_set1_ps(unit), _mm_cvtepi32_ps(_mm_cvtepi16_epi32(input))));
			dst += 4;
			_mm_store_ps(dst, _mm_mul_ps(_mm_set1_ps(unit), _mm_cvtepi32_ps(_mm_cvtepi16_epi32(_mm_srli_si128(input, 8)))));
			dst += 4;
		}
		for (; length > 0; --length) // For some reason it's faster than i-based loop with the preceding SSE-optimized loop, but slower without one.
			*dst++ = static_cast<float>(*src++) * unit;
#else
		for (size_t i = 0; i < length; ++i)
			dst[i] = static_cast<float>(src[i]) * unit;
#endif
	}

	void convertSamples2x1D(float* dst, const int16_t* src, size_t length) noexcept
	{
		constexpr auto unit = 1.f / 32768.f;
#if SEIR_INTRINSICS_SSE // 120-160% faster with MSVC.
		for (; length >= 8; length -= 8)
		{
			const auto input = _mm_load_si128(reinterpret_cast<const __m128i*>(src));
			src += 8;
			const auto normalized1 = _mm_mul_ps(_mm_set1_ps(unit), _mm_cvtepi32_ps(_mm_cvtepi16_epi32(input)));
			const auto normalized2 = _mm_mul_ps(_mm_set1_ps(unit), _mm_cvtepi32_ps(_mm_cvtepi16_epi32(_mm_srli_si128(input, 8))));
			_mm_store_ps(dst, _mm_unpacklo_ps(normalized1, normalized1));
			dst += 4;
			_mm_store_ps(dst, _mm_unpackhi_ps(normalized1, normalized1));
			dst += 4;
			_mm_store_ps(dst, _mm_unpacklo_ps(normalized2, normalized2));
			dst += 4;
			_mm_store_ps(dst, _mm_unpackhi_ps(normalized2, normalized2));
			dst += 4;
		}
#endif
		for (; length > 0; --length)
		{
			const auto value = static_cast<float>(*src++) * unit;
			*dst++ = value;
			*dst++ = value;
		}
	}

	void duplicate1D_16(void* dst, const void* src, size_t length) noexcept
	{
		size_t i = 0;
#if SEIR_INTRINSICS_SSE // 4-8x faster with MSVC.
		for (; i < (length & ~size_t{ 0b111 }); i += 8)
		{
			const auto block = _mm_load_si128(reinterpret_cast<const __m128i*>(static_cast<const uint16_t*>(src) + i));
			_mm_store_si128(reinterpret_cast<__m128i*>(static_cast<uint16_t*>(dst) + 2 * i), _mm_unpacklo_epi16(block, block));
			_mm_store_si128(reinterpret_cast<__m128i*>(static_cast<uint16_t*>(dst) + 2 * i + 8), _mm_unpackhi_epi16(block, block));
		}
#endif
		for (; i < length; ++i)
		{
			const auto value = static_cast<const uint16_t*>(src)[i]; // This does generate better assembly.
			static_cast<uint16_t*>(dst)[2 * i] = value;
			static_cast<uint16_t*>(dst)[2 * i + 1] = value;
		}
	}

	void duplicate1D_32(void* dst, const void* src, size_t length) noexcept
	{
		size_t i = 0;
#if SEIR_INTRINSICS_SSE // 2-4x faster with MSVC.
		for (; i < (length & ~size_t{ 0b11 }); i += 4)
		{
			const auto block = _mm_load_si128(reinterpret_cast<const __m128i*>(static_cast<const uint32_t*>(src) + i));
			_mm_store_si128(reinterpret_cast<__m128i*>(static_cast<uint32_t*>(dst) + 2 * i), _mm_unpacklo_epi32(block, block));
			_mm_store_si128(reinterpret_cast<__m128i*>(static_cast<uint32_t*>(dst) + 2 * i + 4), _mm_unpackhi_epi32(block, block));
		}
#endif
		for (; i < length; ++i)
		{
			const auto value = static_cast<const uint32_t*>(src)[i]; // This does generate better assembly.
			static_cast<uint32_t*>(dst)[2 * i] = value;
			static_cast<uint32_t*>(dst)[2 * i + 1] = value;
		}
	}
}
