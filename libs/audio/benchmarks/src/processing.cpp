// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_audio/processing.hpp>
#include <seir_base/buffer.hpp>

#include <algorithm>
#include <numeric>

#include <benchmark/benchmark.h>

namespace
{
	void baseline_addSamples1D_i16(float* dst, const int16_t* src, size_t length) noexcept
	{
		constexpr auto unit = 1.f / 32768.f;
		for (size_t i = 0; i < length; ++i)
			dst[i] += static_cast<float>(src[i]) * unit;
	}

	template <typename T, auto function>
	void benchmark_addSamples1D(benchmark::State& state)
	{
		seir::Buffer<T, seir::AlignedAllocator<seir::kAudioAlignment>> src{ static_cast<size_t>(state.range(0) - 1) / sizeof(T) };
		std::iota(src.data(), src.data() + src.capacity(), T{});
		seir::Buffer<float, seir::AlignedAllocator<seir::kAudioAlignment>> dst{ src.capacity() };
		std::iota(dst.data(), dst.data() + dst.capacity(), 0.f);
		for (auto _ : state)
			function(dst.data(), src.data(), src.capacity());
	}

	void addSamples1D_i16_Opt(benchmark::State& state) { benchmark_addSamples1D<int16_t, static_cast<void (*)(float*, const int16_t*, size_t)>(seir::addSamples1D)>(state); }
	void addSamples1D_i16_Ref(benchmark::State& state) { benchmark_addSamples1D<int16_t, baseline_addSamples1D_i16>(state); }
}

BENCHMARK(addSamples1D_i16_Opt)->Arg(seir::kAudioAlignment)->Arg(2 * seir::kAudioAlignment)->RangeMultiplier(4)->Range(1 << 10, 1 << 20);
BENCHMARK(addSamples1D_i16_Ref)->Arg(seir::kAudioAlignment)->Arg(2 * seir::kAudioAlignment)->RangeMultiplier(4)->Range(1 << 10, 1 << 20);

namespace
{
	void baseline_addSamples2x1D_i16(float* dst, const int16_t* src, size_t length) noexcept
	{
		constexpr auto unit = 1.f / 32768.f;
		for (; length > 0; --length)
		{
			const auto value = static_cast<float>(*src++) * unit;
			*dst++ += value;
			*dst++ += value;
		}
	}

	void baseline_addSamples2x1D_f32(float* dst, const float* src, size_t length) noexcept
	{
		for (; length > 0; --length)
		{
			const auto value = *src++;
			*dst++ += value;
			*dst++ += value;
		}
	}

	template <typename T, auto function>
	void benchmark_addSamples2x1D(benchmark::State& state)
	{
		seir::Buffer<T, seir::AlignedAllocator<seir::kAudioAlignment>> src{ static_cast<size_t>(state.range(0) - 1) / sizeof(T) };
		std::iota(src.data(), src.data() + src.capacity(), T{});
		seir::Buffer<float, seir::AlignedAllocator<seir::kAudioAlignment>> dst{ src.capacity() * 2 };
		std::iota(dst.data(), dst.data() + dst.capacity(), 0.f);
		for (auto _ : state)
			function(dst.data(), src.data(), src.capacity());
	}

	void addSamples2x1D_i16_Opt(benchmark::State& state) { benchmark_addSamples2x1D<int16_t, static_cast<void (*)(float*, const int16_t*, size_t)>(seir::addSamples2x1D)>(state); }
	void addSamples2x1D_i16_Ref(benchmark::State& state) { benchmark_addSamples2x1D<int16_t, baseline_addSamples2x1D_i16>(state); }
	void addSamples2x1D_f32_Opt(benchmark::State& state) { benchmark_addSamples2x1D<float, static_cast<void (*)(float*, const float*, size_t)>(seir::addSamples2x1D)>(state); }
	void addSamples2x1D_f32_Ref(benchmark::State& state) { benchmark_addSamples2x1D<float, baseline_addSamples2x1D_f32>(state); }
}

BENCHMARK(addSamples2x1D_i16_Opt)->Arg(seir::kAudioAlignment)->Arg(2 * seir::kAudioAlignment)->RangeMultiplier(4)->Range(1 << 10, 1 << 20);
BENCHMARK(addSamples2x1D_i16_Ref)->Arg(seir::kAudioAlignment)->Arg(2 * seir::kAudioAlignment)->RangeMultiplier(4)->Range(1 << 10, 1 << 20);
BENCHMARK(addSamples2x1D_f32_Opt)->Arg(seir::kAudioAlignment)->Arg(2 * seir::kAudioAlignment)->RangeMultiplier(4)->Range(1 << 10, 1 << 20);
BENCHMARK(addSamples2x1D_f32_Ref)->Arg(seir::kAudioAlignment)->Arg(2 * seir::kAudioAlignment)->RangeMultiplier(4)->Range(1 << 10, 1 << 20);

namespace
{
	void baseline_convertSamples1D_i16(float* dst, const int16_t* src, size_t length) noexcept
	{
		constexpr auto unit = 1.f / 32768.f;
		for (size_t i = 0; i < length; ++i)
			dst[i] = static_cast<float>(src[i]) * unit;
	}

	template <typename T, auto function>
	void benchmark_convertSamples1D(benchmark::State& state)
	{
		seir::Buffer<T, seir::AlignedAllocator<seir::kAudioAlignment>> src{ static_cast<size_t>(state.range(0) - 1) / sizeof(T) };
		std::iota(src.data(), src.data() + src.capacity(), T{});
		seir::Buffer<float, seir::AlignedAllocator<seir::kAudioAlignment>> dst{ src.capacity() };
		std::iota(dst.data(), dst.data() + dst.capacity(), 0.f);
		for (auto _ : state)
			function(dst.data(), src.data(), src.capacity());
	}

	void convertSamples1D_i16_Opt(benchmark::State& state) { benchmark_convertSamples1D<int16_t, static_cast<void (*)(float*, const int16_t*, size_t)>(seir::convertSamples1D)>(state); }
	void convertSamples1D_i16_Ref(benchmark::State& state) { benchmark_convertSamples1D<int16_t, baseline_convertSamples1D_i16>(state); }
}

BENCHMARK(convertSamples1D_i16_Opt)->Arg(seir::kAudioAlignment)->Arg(2 * seir::kAudioAlignment)->RangeMultiplier(4)->Range(1 << 10, 1 << 20);
BENCHMARK(convertSamples1D_i16_Ref)->Arg(seir::kAudioAlignment)->Arg(2 * seir::kAudioAlignment)->RangeMultiplier(4)->Range(1 << 10, 1 << 20);

namespace
{
	void baseline_convertSamples2x1D_i16(float* dst, const int16_t* src, size_t length) noexcept
	{
		constexpr auto unit = 1.f / 32768.f;
		for (; length > 0; --length)
		{
			const auto value = static_cast<float>(*src++) * unit;
			*dst++ = value;
			*dst++ = value;
		}
	}

	template <typename T, auto function>
	void benchmark_convertSamples2x1D(benchmark::State& state)
	{
		seir::Buffer<T, seir::AlignedAllocator<seir::kAudioAlignment>> src{ static_cast<size_t>(state.range(0) - 1) / sizeof(T) };
		std::iota(src.data(), src.data() + src.capacity(), T{});
		seir::Buffer<float, seir::AlignedAllocator<seir::kAudioAlignment>> dst{ src.capacity() * 2 };
		std::iota(dst.data(), dst.data() + dst.capacity(), 0.f);
		for (auto _ : state)
			function(dst.data(), src.data(), src.capacity());
	}

	void convertSamples2x1D_i16_Opt(benchmark::State& state) { benchmark_convertSamples2x1D<int16_t, static_cast<void (*)(float*, const int16_t*, size_t)>(seir::convertSamples2x1D)>(state); }
	void convertSamples2x1D_i16_Ref(benchmark::State& state) { benchmark_convertSamples2x1D<int16_t, baseline_convertSamples2x1D_i16>(state); }
}

BENCHMARK(convertSamples2x1D_i16_Opt)->Arg(seir::kAudioAlignment)->Arg(2 * seir::kAudioAlignment)->RangeMultiplier(4)->Range(1 << 10, 1 << 20);
BENCHMARK(convertSamples2x1D_i16_Ref)->Arg(seir::kAudioAlignment)->Arg(2 * seir::kAudioAlignment)->RangeMultiplier(4)->Range(1 << 10, 1 << 20);

namespace
{
	void baseline_duplicate1D_i16(int16_t* dst, const int16_t* src, size_t length) noexcept
	{
		for (; length > 0; --length)
		{
			const auto value = *src++;
			*dst++ = value;
			*dst++ = value;
		}
	}

	void baseline_duplicate1D_i32(int32_t* dst, const int32_t* src, size_t srcLength) noexcept
	{
		for (; srcLength > 0; --srcLength)
		{
			const auto value = *src++;
			*dst++ = value;
			*dst++ = value;
		}
	}

	template <typename T, auto function>
	void benchmark_duplicate1D(benchmark::State& state)
	{
		seir::Buffer<T, seir::AlignedAllocator<seir::kAudioAlignment>> src{ static_cast<size_t>(state.range(0) - 1) / sizeof(T) };
		std::iota(src.data(), src.data() + src.capacity(), T{});
		seir::Buffer<T, seir::AlignedAllocator<seir::kAudioAlignment>> dst{ src.capacity() * 2 };
		for (auto _ : state)
			function(dst.data(), src.data(), src.capacity());
	}

	void duplicate1D_i16_Opt(benchmark::State& state) { benchmark_duplicate1D<int16_t, seir::duplicate1D_16>(state); }
	void duplicate1D_i16_Ref(benchmark::State& state) { benchmark_duplicate1D<int16_t, baseline_duplicate1D_i16>(state); }
	void duplicate1D_i32_Opt(benchmark::State& state) { benchmark_duplicate1D<int32_t, seir::duplicate1D_32>(state); }
	void duplicate1D_i32_Ref(benchmark::State& state) { benchmark_duplicate1D<int32_t, baseline_duplicate1D_i32>(state); }
}

BENCHMARK(duplicate1D_i16_Opt)->Arg(seir::kAudioAlignment)->Arg(2 * seir::kAudioAlignment)->RangeMultiplier(4)->Range(1 << 10, 1 << 20);
BENCHMARK(duplicate1D_i16_Ref)->Arg(seir::kAudioAlignment)->Arg(2 * seir::kAudioAlignment)->RangeMultiplier(4)->Range(1 << 10, 1 << 20);
BENCHMARK(duplicate1D_i32_Opt)->Arg(seir::kAudioAlignment)->Arg(2 * seir::kAudioAlignment)->RangeMultiplier(4)->Range(1 << 10, 1 << 20);
BENCHMARK(duplicate1D_i32_Ref)->Arg(seir::kAudioAlignment)->Arg(2 * seir::kAudioAlignment)->RangeMultiplier(4)->Range(1 << 10, 1 << 20);
