// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "../../src/common.hpp"
#include "../../src/processing.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <numeric>
#include <utility>

#include <doctest/doctest.h>

namespace
{
	constexpr auto sentinelFloat = 4.f;

	template <typename T, size_t N>
	constexpr bool checkSize(const std::array<T, N>&) noexcept
	{
		return N * sizeof(T) > 2 * seir::kAudioBlockAlignment // To be able to check every possible remainder length.
			&& (seir::kAudioBlockAlignment < sizeof(T)
				|| N * sizeof(T) % seir::kAudioBlockAlignment == sizeof(T)); // To check we're not triggering ASAN.
	}

	template <typename T>
	constexpr size_t alignOf = std::max(seir::kAudioBlockAlignment, alignof(T));
}

TEST_CASE("addSamples1D")
{
	SUBCASE("float")
	{
		alignas(::alignOf<float>) const std::array<float, 17> src{
			-1.f, -.875f, -.75f, -.625f, -.5f, -.375f, -.25f, -.125f,
			0.f, .125f, .25f, .375f, .5f, .625f, .75f, .875f, 1.f
		};
		static_assert(::checkSize(src));
		alignas(::alignOf<float>) std::array<float, src.size()> dst{};
		const std::array<float, dst.size()> expected{
			-1.875f, -1.625f, -1.375f, -1.125f, -.875f, -.625f, -.375f, -.125f,
			.125f, .375f, .625f, .875f, 1.125f, 1.375f, 1.625f, 1.875f, 2.125f
		};
		for (auto size = src.size(); size >= src.size() - seir::kAudioBlockAlignment / sizeof src[0]; --size)
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
		alignas(::alignOf<int16_t>) const std::array<int16_t, 17> src{
			-32768, -28672, -24576, -20480, -16384, -12288, -8192, -4096,
			0, 4096, 8192, 12288, 16384, 20480, 24576, 28672, 30720
		};
		static_assert(::checkSize(src));
		alignas(::alignOf<float>) std::array<float, src.size()> dst{};
		const std::array<float, dst.size()> expected{
			-1.875f, -1.625f, -1.375f, -1.125f, -.875f, -.625f, -.375f, -.125f,
			.125f, .375f, .625f, .875f, 1.125f, 1.375f, 1.625f, 1.875f, 2.0625f
		};
		for (auto size = src.size(); size >= src.size() - seir::kAudioBlockAlignment / sizeof src[0]; --size)
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
		alignas(::alignOf<float>) const std::array<float, 17> src{
			-1.f, -.875f, -.75f, -.625f, -.5f, -.375f, -.25f, -.125f,
			0.f, .125f, .25f, .375f, .5f, .625f, .75f, .875f, 1.f
		};
		static_assert(::checkSize(src));
		alignas(::alignOf<float>) std::array<float, src.size() * 2> dst{};
		const std::array<float, dst.size()> expected{
			-1.875f, -1.875f, -1.625f, -1.625f, -1.375f, -1.375f, -1.125f, -1.125f,
			-.875f, -.875f, -.625f, -.625f, -.375f, -.375f, -.125f, -.125f,
			.125f, .125f, .375f, .375f, .625f, .625f, .875f, .875f, 1.125f,
			1.125f, 1.375f, 1.375f, 1.625f, 1.625f, 1.875f, 1.875f, 2.125f, 2.125f
		};
		for (auto size = src.size(); size >= src.size() - seir::kAudioBlockAlignment / sizeof src[0]; --size)
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
		alignas(::alignOf<int16_t>) const std::array<int16_t, 17> src{
			-32768, -28672, -24576, -20480, -16384, -12288, -8192, -4096,
			0, 4096, 8192, 12288, 16384, 20480, 24576, 28672, 30720
		};
		static_assert(::checkSize(src));
		alignas(::alignOf<float>) std::array<float, src.size() * 2> dst{};
		const std::array<float, dst.size()> expected{
			-1.875f, -1.875f, -1.625f, -1.625f, -1.375f, -1.375f, -1.125f, -1.125f,
			-.875f, -.875f, -.625f, -.625f, -.375f, -.375f, -.125f, -.125f,
			.125f, .125f, .375f, .375f, .625f, .625f, .875f, .875f, 1.125f,
			1.125f, 1.375f, 1.375f, 1.625f, 1.625f, 1.875f, 1.875f, 2.0625f, 2.0625f
		};
		for (auto size = src.size(); size >= src.size() - seir::kAudioBlockAlignment / sizeof src[0]; --size)
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

TEST_CASE("convertSamples1D")
{
	SUBCASE("int16_t")
	{
		alignas(::alignOf<int16_t>) const std::array<int16_t, 17> src{
			-32768, -28672, -24576, -20480, -16384, -12288, -8192, -4096,
			0, 4096, 8192, 12288, 16384, 20480, 24576, 28672, 30720
		};
		static_assert(::checkSize(src));
		alignas(::alignOf<float>) std::array<float, src.size()> dst{};
		const std::array<float, dst.size()> expected{
			-1.f, -.875f, -.75f, -.625f, -.5f, -.375f, -.25f, -.125f,
			0.f, .125f, .25f, .375f, .5f, .625f, .75f, .875f, .9375f
		};
		for (auto size = src.size(); size >= src.size() - seir::kAudioBlockAlignment / sizeof src[0]; --size)
		{
			INFO("size = " << size);
			std::generate_n(dst.begin(), size, [value = -1.f]() mutable { return value += .125f; });
			std::fill_n(dst.begin() + static_cast<ptrdiff_t>(size), dst.size() - size, sentinelFloat);
			seir::convertSamples1D(dst.data(), src.data(), size);
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

TEST_CASE("convertSamples2x1D")
{
	SUBCASE("int16_t")
	{
		alignas(::alignOf<int16_t>) const std::array<int16_t, 17> src{
			-32768, -28672, -24576, -20480, -16384, -12288, -8192, -4096,
			0, 4096, 8192, 12288, 16384, 20480, 24576, 28672, 30720
		};
		static_assert(::checkSize(src));
		alignas(::alignOf<float>) std::array<float, src.size() * 2> dst{};
		const std::array<float, dst.size()> expected{
			-1.f, -1.f, -.875f, -.875f, -.75f, -.75f, -.625f, -.625f,
			-.5f, -.5f, -.375f, -.375f, -.25f, -.25f, -.125f, -.125f,
			0.f, 0.f, .125f, .125f, .25f, .25f, .375f, .375f,
			.5f, .5f, .625f, .625f, .75f, .75f, .875f, .875f, .9375f, .9375f
		};
		for (auto size = src.size(); size >= src.size() - seir::kAudioBlockAlignment / sizeof src[0]; --size)
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
			seir::convertSamples2x1D(dst.data(), src.data(), size);
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

TEST_CASE("duplicate1D")
{
	SUBCASE("16")
	{
		alignas(::alignOf<int16_t>) const std::array<int16_t, 17> src{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };
		static_assert(::checkSize(src));
		alignas(::alignOf<int16_t>) std::array<int16_t, src.size() * 2> dst{};
		for (auto size = src.size(); size >= src.size() - seir::kAudioBlockAlignment / sizeof src[0]; --size)
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
		alignas(::alignOf<int32_t>) const std::array<int32_t, 9> src{ 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		static_assert(::checkSize(src));
		alignas(::alignOf<int32_t>) std::array<int32_t, src.size() * 2> dst{};
		for (auto size = src.size(); size >= src.size() - seir::kAudioBlockAlignment / sizeof src[0]; --size)
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

TEST_CASE("resample*2x1D")
{
	SUBCASE("upsampling")
	{
		constexpr size_t srcFrames = 5;
		constexpr size_t dstFrames = 13;
		alignas(::alignOf<float>) const std::array<float, 2 * srcFrames> src{
			0.f, 1.f,
			2.f, 3.f,
			4.f, 5.f,
			6.f, 7.f,
			8.f, 9.f
		};
		decltype(&seir::resampleAdd2x1D) function = nullptr;
		std::array<float, 2 * dstFrames> expected;
		SUBCASE("add")
		{
			function = seir::resampleAdd2x1D;
			expected = std::to_array<float>({
				0.00f, 1.25f, 0.50f, 1.75f, 0.00f, 1.25f, // 0 + { 0/13, 5/13, 10/13 }
				2.50f, 3.75f, 2.00f, 3.25f, 2.50f, 3.75f, // 1 + { 2/13, 7/13, 12/13 }
				4.00f, 5.25f, 4.50f, 5.75f,               // 2 + { 4/13, 9/13 }
				6.00f, 7.25f, 6.50f, 7.75f, 6.00f, 7.25f, // 3 + { 1/13, 6/13, 11/13 }
				8.50f, 9.75f, 8.00f, 9.25f                // 4 + { 3/13, 8/13 }
			});
		}
		SUBCASE("copy")
		{
			function = seir::resampleCopy2x1D;
			expected = std::to_array<float>({
				0.f, 1.f, 0.f, 1.f, 0.f, 1.f, // 0 + { 0/13, 5/13, 10/13 }
				2.f, 3.f, 2.f, 3.f, 2.f, 3.f, // 1 + { 2/13, 7/13, 12/13 }
				4.f, 5.f, 4.f, 5.f,           // 2 + { 4/13, 9/13 }
				6.f, 7.f, 6.f, 7.f, 6.f, 7.f, // 3 + { 1/13, 6/13, 11/13 }
				8.f, 9.f, 8.f, 9.f            // 4 + { 3/13, 8/13 }
			});
		}
		alignas(::alignOf<float>) std::array<float, expected.size()> dst{};
		constexpr auto step = (srcFrames << seir::kAudioResamplingFractionBits) / dstFrames;
		for (auto frames = dstFrames; frames > 0; --frames)
		{
			INFO("frames = " << frames);
			std::fill(
				std::generate_n(dst.begin(), 2 * frames, [i = 0.f, dummy = 0.f]() mutable {
					return std::exchange(i, std::modf(i + .25f, &dummy));
				}),
				dst.end(), sentinelFloat);
			function(dst.data(), frames, src.data(), 0, step);
			for (size_t i = 0; i < frames * 2; ++i)
			{
				INFO("i = " << i);
				CHECK(dst[i] == expected[i]);
			}
			for (auto i = frames * 2; i < dst.size(); ++i)
			{
				INFO("i = " << i);
				CHECK(dst[i] == sentinelFloat);
			}
		}
	}
	SUBCASE("downsampling")
	{
		constexpr size_t srcFrames = 13;
		constexpr size_t dstFrames = 5;
		alignas(::alignOf<float>) const std::array<float, 2 * srcFrames> src{
			0.f, 1.f,
			2.f, 3.f,
			4.f, 5.f,
			6.f, 7.f,
			8.f, 9.f,
			10.f, 11.f,
			12.f, 13.f,
			14.f, 15.f,
			16.f, 17.f,
			18.f, 19.f,
			20.f, 21.f,
			22.f, 23.f,
			24.f, 25.f
		};
		decltype(&seir::resampleAdd2x1D) function = nullptr;
		std::array<float, 2 * dstFrames> expected;
		SUBCASE("add")
		{
			function = seir::resampleAdd2x1D;
			expected = std::to_array<float>({
				0.00f, 1.25f,   // 0/5 = 0.0
				4.50f, 5.75f,   // 13/5 = 2.6
				10.00f, 11.25f, // 26/5 = 5.2
				14.50f, 15.75f, // 39/5 = 7.8
				20.00f, 21.25f  // 52/5 = 10.4
			});
		}
		SUBCASE("copy")
		{
			function = seir::resampleCopy2x1D;
			expected = std::to_array<float>({
				0.f, 1.f,   // 0/5 = 0.0
				4.f, 5.f,   // 13/5 = 2.6
				10.f, 11.f, // 26/5 = 5.2
				14.f, 15.f, // 39/5 = 7.8
				20.f, 21.f  // 52/5 = 10.4
			});
		}
		alignas(::alignOf<float>) std::array<float, expected.size()> dst{};
		constexpr auto step = (srcFrames << seir::kAudioResamplingFractionBits) / dstFrames;
		for (auto frames = dstFrames; frames > 0; --frames)
		{
			INFO("frames = " << frames);
			std::fill(
				std::generate_n(dst.begin(), 2 * frames, [i = 0.f, dummy = 0.f]() mutable {
					return std::exchange(i, std::modf(i + .25f, &dummy));
				}),
				dst.end(), sentinelFloat);
			function(dst.data(), frames, src.data(), 0, step);
			for (size_t i = 0; i < frames * 2; ++i)
			{
				INFO("i = " << i);
				CHECK(dst[i] == expected[i]);
			}
			for (auto i = frames * 2; i < dst.size(); ++i)
			{
				INFO("i = " << i);
				CHECK(dst[i] == sentinelFloat);
			}
		}
	}
}
