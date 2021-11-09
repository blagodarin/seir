// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_audio/processing.hpp>

#include <algorithm>
#include <array>
#include <numeric>

#include <doctest/doctest.h>

namespace
{
	constexpr auto sentinelFloat = 4.f;

	template <typename T, size_t N>
	constexpr bool checkSize(const std::array<T, N>&) noexcept
	{
		return N * sizeof(T) > 2 * seir::kAudioAlignment           // To be able to check every possible remainder length.
			&& N * sizeof(T) % seir::kAudioAlignment == sizeof(T); // To check we're not triggering ASAN.
	}
}

TEST_CASE("addSamples1D")
{
	SUBCASE("float")
	{
		alignas(seir::kAudioAlignment) const std::array<float, 17> src{
			-1.f, -.875f, -.75f, -.625f, -.5f, -.375f, -.25f, -.125f,
			0.f, .125f, .25f, .375f, .5f, .625f, .75f, .875f, 1.f
		};
		static_assert(::checkSize(src));
		alignas(seir::kAudioAlignment) std::array<float, src.size()> dst{};
		const std::array<float, dst.size()> expected{
			-1.875f, -1.625f, -1.375f, -1.125f, -.875f, -.625f, -.375f, -.125f,
			.125f, .375f, .625f, .875f, 1.125f, 1.375f, 1.625f, 1.875f, 2.125f
		};
		for (auto size = src.size(); size >= src.size() - seir::kAudioAlignment / sizeof src[0]; --size)
		{
			INFO("size = " << size);
			std::generate_n(dst.begin(), size, [value = -1.f]() mutable { return value += .125f; });
			std::fill_n(dst.begin() + static_cast<ptrdiff_t>(size), dst.size() - size, sentinelFloat);
			seir::addSamples1D(dst.data(), src.data(), size);
			for (size_t i = 0; i < size; ++i)
			{
				INFO("i = " << i);
				CHECK(dst[i] == expected[i]);
			}
			for (auto i = size; i < dst.size(); ++i)
			{
				INFO("i = " << i);
				CHECK(dst[i] == sentinelFloat);
			}
		}
	}
	SUBCASE("int16_t")
	{
		alignas(seir::kAudioAlignment) const std::array<int16_t, 17> src{
			-32768, -28672, -24576, -20480, -16384, -12288, -8192, -4096,
			0, 4096, 8192, 12288, 16384, 20480, 24576, 28672, 30720
		};
		static_assert(::checkSize(src));
		alignas(seir::kAudioAlignment) std::array<float, src.size()> dst{};
		const std::array<float, dst.size()> expected{
			-1.875f, -1.625f, -1.375f, -1.125f, -.875f, -.625f, -.375f, -.125f,
			.125f, .375f, .625f, .875f, 1.125f, 1.375f, 1.625f, 1.875f, 2.0625f
		};
		for (auto size = src.size(); size >= src.size() - seir::kAudioAlignment / sizeof src[0]; --size)
		{
			INFO("size = " << size);
			std::generate_n(dst.begin(), size, [value = -1.f]() mutable { return value += .125f; });
			std::fill_n(dst.begin() + static_cast<ptrdiff_t>(size), dst.size() - size, sentinelFloat);
			seir::addSamples1D(dst.data(), src.data(), size);
			for (size_t i = 0; i < size; ++i)
			{
				INFO("i = " << i);
				CHECK(dst[i] == expected[i]);
			}
			for (auto i = size; i < dst.size(); ++i)
			{
				INFO("i = " << i);
				CHECK(dst[i] == sentinelFloat);
			}
		}
	}
}

TEST_CASE("addSamples2x1D")
{
	SUBCASE("float")
	{
		alignas(seir::kAudioAlignment) const std::array<float, 17> src{
			-1.f, -.875f, -.75f, -.625f, -.5f, -.375f, -.25f, -.125f,
			0.f, .125f, .25f, .375f, .5f, .625f, .75f, .875f, 1.f
		};
		static_assert(::checkSize(src));
		alignas(seir::kAudioAlignment) std::array<float, src.size() * 2> dst{};
		const std::array<float, dst.size()> expected{
			-1.875f, -1.875f, -1.625f, -1.625f, -1.375f, -1.375f, -1.125f, -1.125f,
			-.875f, -.875f, -.625f, -.625f, -.375f, -.375f, -.125f, -.125f,
			.125f, .125f, .375f, .375f, .625f, .625f, .875f, .875f, 1.125f,
			1.125f, 1.375f, 1.375f, 1.625f, 1.625f, 1.875f, 1.875f, 2.125f, 2.125f
		};
		for (auto size = src.size(); size >= src.size() - seir::kAudioAlignment / sizeof src[0]; --size)
		{
			INFO("size = " << size);
			const auto dstSize = size * 2;
			std::generate_n(dst.begin(), dstSize, [value = -1.f, increment = true]() mutable {
				if (increment)
					value += .125f;
				increment = !increment;
				return value;
			});
			std::fill_n(dst.begin() + static_cast<ptrdiff_t>(dstSize), dst.size() - dstSize, sentinelFloat);
			seir::addSamples2x1D(dst.data(), src.data(), size);
			for (size_t i = 0; i < dstSize; ++i)
			{
				INFO("i = " << i);
				CHECK(dst[i] == expected[i]);
			}
			for (auto i = dstSize; i < dst.size(); ++i)
			{
				INFO("i = " << i);
				CHECK(dst[i] == sentinelFloat);
			}
		}
	}
	SUBCASE("int16_t")
	{
		alignas(seir::kAudioAlignment) const std::array<int16_t, 17> src{
			-32768, -28672, -24576, -20480, -16384, -12288, -8192, -4096,
			0, 4096, 8192, 12288, 16384, 20480, 24576, 28672, 30720
		};
		static_assert(::checkSize(src));
		alignas(seir::kAudioAlignment) std::array<float, src.size() * 2> dst{};
		const std::array<float, dst.size()> expected{
			-1.875f, -1.875f, -1.625f, -1.625f, -1.375f, -1.375f, -1.125f, -1.125f,
			-.875f, -.875f, -.625f, -.625f, -.375f, -.375f, -.125f, -.125f,
			.125f, .125f, .375f, .375f, .625f, .625f, .875f, .875f, 1.125f,
			1.125f, 1.375f, 1.375f, 1.625f, 1.625f, 1.875f, 1.875f, 2.0625f, 2.0625f
		};
		for (auto size = src.size(); size >= src.size() - seir::kAudioAlignment / sizeof src[0]; --size)
		{
			INFO("size = " << size);
			const auto dstSize = size * 2;
			std::generate_n(dst.begin(), dstSize, [value = -1.f, increment = true]() mutable {
				if (increment)
					value += .125f;
				increment = !increment;
				return value;
			});
			std::fill_n(dst.begin() + static_cast<ptrdiff_t>(dstSize), dst.size() - dstSize, sentinelFloat);
			seir::addSamples2x1D(dst.data(), src.data(), size);
			for (size_t i = 0; i < dstSize; ++i)
			{
				INFO("i = " << i);
				CHECK(dst[i] == expected[i]);
			}
			for (auto i = dstSize; i < dst.size(); ++i)
			{
				INFO("i = " << i);
				CHECK(dst[i] == sentinelFloat);
			}
		}
	}
}

TEST_CASE("duplicate1D_16")
{
	SUBCASE("16")
	{
		alignas(seir::kAudioAlignment) const std::array<int16_t, 17> src{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };
		static_assert(::checkSize(src));
		alignas(seir::kAudioAlignment) std::array<int16_t, src.size() * 2> dst{};
		for (auto size = src.size(); size >= src.size() - seir::kAudioAlignment / sizeof src[0]; --size)
		{
			INFO("size = " << size);
			std::fill(dst.begin(), dst.end(), int16_t{ 0 });
			seir::duplicate1D_16(dst.data(), src.data(), size);
			for (size_t i = 0; i < size * 2; ++i)
			{
				INFO("i = " << i);
				CHECK(dst[i] == src[i / 2]);
			}
			for (auto i = size * 2; i < dst.size(); ++i)
			{
				INFO("i = " << i);
				CHECK(dst[i] == 0);
			}
		}
	}
	SUBCASE("32")
	{
		alignas(seir::kAudioAlignment) const std::array<int32_t, 9> src{ 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		static_assert(::checkSize(src));
		alignas(seir::kAudioAlignment) std::array<int32_t, src.size() * 2> dst{};
		for (auto size = src.size(); size >= src.size() - seir::kAudioAlignment / sizeof src[0]; --size)
		{
			INFO("size = " << size);
			std::fill(dst.begin(), dst.end(), int32_t{ 0 });
			seir::duplicate1D_32(dst.data(), src.data(), size);
			for (size_t i = 0; i < size * 2; ++i)
			{
				INFO("i = " << i);
				CHECK(dst[i] == src[i / 2]);
			}
			for (auto i = size * 2; i < dst.size(); ++i)
			{
				INFO("i = " << i);
				CHECK(dst[i] == 0);
			}
		}
	}
}
