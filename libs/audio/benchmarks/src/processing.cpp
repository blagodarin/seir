// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_base/allocator.hpp>
#include "../../src/common.hpp"
#include "../../src/processing.hpp"

#include <algorithm>
#include <iterator>

#include <benchmark/benchmark.h>

namespace
{
	template <class T>
	class StdAllocator
	{
	public:
		using value_type = T;

		constexpr StdAllocator() noexcept = default;

		template <class U>
		constexpr StdAllocator(const StdAllocator<U>&) noexcept {}

		[[nodiscard]] T* allocate(size_t n)
		{
			if (n > std::numeric_limits<size_t>::max() / sizeof(T))
				throw std::bad_array_new_length{};
			auto capacityBytes = n * sizeof(T);
			return static_cast<T*>(Allocator::allocate(capacityBytes));
		}

		void deallocate(T* p, size_t) noexcept
		{
			Allocator::deallocate(p);
		}

	private:
		using Allocator = seir::AlignedAllocator<seir::kAudioBlockAlignment>;
	};

	template <class T>
	using StdVector = std::vector<T, StdAllocator<T>>;

	template <class Dst, class Src>
	struct Buffers
	{
		StdVector<Dst> _dst;
		StdVector<Src> _src;

		Buffers(const benchmark::State& state, size_t dstSizeFactor = 1)
		{
			const auto srcSize = static_cast<size_t>(state.range(0) - 1) / sizeof(Src);
			_src.reserve(srcSize);
			std::generate_n(std::back_inserter(_src), srcSize, [i = Src{}]() mutable { return ++i; });
			_dst.reserve(srcSize * dstSizeFactor);
			std::generate_n(std::back_inserter(_dst), srcSize * dstSizeFactor, [i = Dst{}]() mutable { return ++i; });
		}

		[[nodiscard]] auto dst() noexcept { return _dst.data(); }
		[[nodiscard]] auto dstSize() const noexcept { return _dst.size(); }
		[[nodiscard]] auto src() noexcept { return _src.data(); }
		[[nodiscard]] auto srcSize() const noexcept { return _src.size(); }
	};
}

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
		Buffers<float, T> buffers{ state };
		for (auto _ : state)
			function(buffers.dst(), buffers.src(), buffers.srcSize());
	}

	void addSamples1D_i16_Opt(benchmark::State& state) { benchmark_addSamples1D<int16_t, static_cast<void (*)(float*, const int16_t*, size_t)>(seir::addSamples1D)>(state); }
	void addSamples1D_i16_Ref(benchmark::State& state) { benchmark_addSamples1D<int16_t, baseline_addSamples1D_i16>(state); }
}

BENCHMARK(addSamples1D_i16_Opt)->Arg(seir::kAudioBlockAlignment)->Arg(2 * seir::kAudioBlockAlignment)->RangeMultiplier(4)->Range(1 << 10, 1 << 20);
BENCHMARK(addSamples1D_i16_Ref)->Arg(seir::kAudioBlockAlignment)->Arg(2 * seir::kAudioBlockAlignment)->RangeMultiplier(4)->Range(1 << 10, 1 << 20);

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
		Buffers<float, T> buffers{ state, 2 };
		for (auto _ : state)
			function(buffers.dst(), buffers.src(), buffers.srcSize());
	}

	void addSamples2x1D_i16_Opt(benchmark::State& state) { benchmark_addSamples2x1D<int16_t, static_cast<void (*)(float*, const int16_t*, size_t)>(seir::addSamples2x1D)>(state); }
	void addSamples2x1D_i16_Ref(benchmark::State& state) { benchmark_addSamples2x1D<int16_t, baseline_addSamples2x1D_i16>(state); }
	void addSamples2x1D_f32_Opt(benchmark::State& state) { benchmark_addSamples2x1D<float, static_cast<void (*)(float*, const float*, size_t)>(seir::addSamples2x1D)>(state); }
	void addSamples2x1D_f32_Ref(benchmark::State& state) { benchmark_addSamples2x1D<float, baseline_addSamples2x1D_f32>(state); }
}

BENCHMARK(addSamples2x1D_i16_Opt)->Arg(seir::kAudioBlockAlignment)->Arg(2 * seir::kAudioBlockAlignment)->RangeMultiplier(4)->Range(1 << 10, 1 << 20);
BENCHMARK(addSamples2x1D_i16_Ref)->Arg(seir::kAudioBlockAlignment)->Arg(2 * seir::kAudioBlockAlignment)->RangeMultiplier(4)->Range(1 << 10, 1 << 20);
BENCHMARK(addSamples2x1D_f32_Opt)->Arg(seir::kAudioBlockAlignment)->Arg(2 * seir::kAudioBlockAlignment)->RangeMultiplier(4)->Range(1 << 10, 1 << 20);
BENCHMARK(addSamples2x1D_f32_Ref)->Arg(seir::kAudioBlockAlignment)->Arg(2 * seir::kAudioBlockAlignment)->RangeMultiplier(4)->Range(1 << 10, 1 << 20);

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
		Buffers<float, T> buffers{ state };
		for (auto _ : state)
			function(buffers.dst(), buffers.src(), buffers.srcSize());
	}

	void convertSamples1D_i16_Opt(benchmark::State& state) { benchmark_convertSamples1D<int16_t, static_cast<void (*)(float*, const int16_t*, size_t)>(seir::convertSamples1D)>(state); }
	void convertSamples1D_i16_Ref(benchmark::State& state) { benchmark_convertSamples1D<int16_t, baseline_convertSamples1D_i16>(state); }
}

BENCHMARK(convertSamples1D_i16_Opt)->Arg(seir::kAudioBlockAlignment)->Arg(2 * seir::kAudioBlockAlignment)->RangeMultiplier(4)->Range(1 << 10, 1 << 20);
BENCHMARK(convertSamples1D_i16_Ref)->Arg(seir::kAudioBlockAlignment)->Arg(2 * seir::kAudioBlockAlignment)->RangeMultiplier(4)->Range(1 << 10, 1 << 20);

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
		Buffers<float, T> buffers{ state, 2 };
		for (auto _ : state)
			function(buffers.dst(), buffers.src(), buffers.srcSize());
	}

	void convertSamples2x1D_i16_Opt(benchmark::State& state) { benchmark_convertSamples2x1D<int16_t, static_cast<void (*)(float*, const int16_t*, size_t)>(seir::convertSamples2x1D)>(state); }
	void convertSamples2x1D_i16_Ref(benchmark::State& state) { benchmark_convertSamples2x1D<int16_t, baseline_convertSamples2x1D_i16>(state); }
}

BENCHMARK(convertSamples2x1D_i16_Opt)->Arg(seir::kAudioBlockAlignment)->Arg(2 * seir::kAudioBlockAlignment)->RangeMultiplier(4)->Range(1 << 10, 1 << 20);
BENCHMARK(convertSamples2x1D_i16_Ref)->Arg(seir::kAudioBlockAlignment)->Arg(2 * seir::kAudioBlockAlignment)->RangeMultiplier(4)->Range(1 << 10, 1 << 20);

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
		Buffers<T, T> buffers{ state, 2 };
		for (auto _ : state)
			function(buffers.dst(), buffers.src(), buffers.srcSize());
	}

	void duplicate1D_i16_Opt(benchmark::State& state) { benchmark_duplicate1D<int16_t, seir::duplicate1D_16>(state); }
	void duplicate1D_i16_Ref(benchmark::State& state) { benchmark_duplicate1D<int16_t, baseline_duplicate1D_i16>(state); }
	void duplicate1D_i32_Opt(benchmark::State& state) { benchmark_duplicate1D<int32_t, seir::duplicate1D_32>(state); }
	void duplicate1D_i32_Ref(benchmark::State& state) { benchmark_duplicate1D<int32_t, baseline_duplicate1D_i32>(state); }
}

BENCHMARK(duplicate1D_i16_Opt)->Arg(seir::kAudioBlockAlignment)->Arg(2 * seir::kAudioBlockAlignment)->RangeMultiplier(4)->Range(1 << 10, 1 << 20);
BENCHMARK(duplicate1D_i16_Ref)->Arg(seir::kAudioBlockAlignment)->Arg(2 * seir::kAudioBlockAlignment)->RangeMultiplier(4)->Range(1 << 10, 1 << 20);
BENCHMARK(duplicate1D_i32_Opt)->Arg(seir::kAudioBlockAlignment)->Arg(2 * seir::kAudioBlockAlignment)->RangeMultiplier(4)->Range(1 << 10, 1 << 20);
BENCHMARK(duplicate1D_i32_Ref)->Arg(seir::kAudioBlockAlignment)->Arg(2 * seir::kAudioBlockAlignment)->RangeMultiplier(4)->Range(1 << 10, 1 << 20);

namespace
{
	void baseline_resampleAdd2x1D(float* dst, size_t dstLength, const float* src, size_t srcOffset, size_t srcStep) noexcept
	{
		for (size_t i = 0, j = srcOffset; i < dstLength; ++i, j += srcStep)
		{
			dst[2 * i] += src[2 * (j >> seir::kAudioResamplingFractionBits)];
			dst[2 * i + 1] += src[2 * (j >> seir::kAudioResamplingFractionBits) + 1];
		}
	}

	template <auto function>
	void benchmark_resampleAdd2x1D(benchmark::State& state)
	{
		Buffers<float, float> buffers{ state };
		for (auto _ : state)
			function(buffers.dst(), buffers.dstSize() / 2, buffers.src(), 0, (5 << seir::kAudioResamplingFractionBits) / 13);
	}

	void resampleAdd2x1D_Opt(benchmark::State& state) { benchmark_resampleAdd2x1D<seir::resampleAdd2x1D>(state); }
	void resampleAdd2x1D_Ref(benchmark::State& state) { benchmark_resampleAdd2x1D<baseline_resampleAdd2x1D>(state); }
}

BENCHMARK(resampleAdd2x1D_Opt)->Arg(seir::kAudioBlockAlignment)->Arg(2 * seir::kAudioBlockAlignment)->RangeMultiplier(4)->Range(1 << 10, 1 << 20);
BENCHMARK(resampleAdd2x1D_Ref)->Arg(seir::kAudioBlockAlignment)->Arg(2 * seir::kAudioBlockAlignment)->RangeMultiplier(4)->Range(1 << 10, 1 << 20);

namespace
{
	void baseline_resampleCopy2x1D(float* dst, size_t dstLength, const float* src, size_t srcOffset, size_t srcStep) noexcept
	{
		for (size_t i = 0, j = srcOffset; i < dstLength; ++i, j += srcStep)
		{
			dst[2 * i] = src[2 * (j >> seir::kAudioResamplingFractionBits)];
			dst[2 * i + 1] = src[2 * (j >> seir::kAudioResamplingFractionBits) + 1];
		}
	}

	template <auto function>
	void benchmark_resampleCopy2x1D(benchmark::State& state)
	{
		Buffers<float, float> buffers{ state };
		for (auto _ : state)
			function(buffers.dst(), buffers.dstSize() / 2, buffers.src(), 0, (5 << seir::kAudioResamplingFractionBits) / 13);
	}

	void resampleCopy2x1D_Opt(benchmark::State& state) { benchmark_resampleCopy2x1D<seir::resampleCopy2x1D>(state); }
	void resampleCopy2x1D_Ref(benchmark::State& state) { benchmark_resampleCopy2x1D<baseline_resampleCopy2x1D>(state); }
}

BENCHMARK(resampleCopy2x1D_Opt)->Arg(seir::kAudioBlockAlignment)->Arg(2 * seir::kAudioBlockAlignment)->RangeMultiplier(4)->Range(1 << 10, 1 << 20);
BENCHMARK(resampleCopy2x1D_Ref)->Arg(seir::kAudioBlockAlignment)->Arg(2 * seir::kAudioBlockAlignment)->RangeMultiplier(4)->Range(1 << 10, 1 << 20);
