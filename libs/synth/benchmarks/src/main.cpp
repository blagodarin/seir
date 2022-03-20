// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

// TODO: Implement proper benchmarks.

#include <seir_synth/composition.hpp>
#include <seir_synth/format.hpp>
#include <seir_synth/renderer.hpp>

#include <array>
#include <cassert>
#include <cmath>
#include <cstring>
#include <chrono>
#include <filesystem>
#include <functional>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>

namespace
{
	std::pair<std::unique_ptr<char[]>, size_t> load(const std::filesystem::path& path)
	{
		std::ifstream stream{ path, std::ios::binary };
		if (!stream)
		{
			std::cerr << "Failed to open input file\n";
			return {};
		}
		stream.seekg(0, std::ios::end);
		const auto size = static_cast<size_t>(stream.tellg());
		auto result = std::make_unique<char[]>(size + 1);
		stream.seekg(0, std::ios::beg);
		stream.read(result.get(), static_cast<std::streamsize>(size));
		result[size] = '\0';
		return { std::move(result), size };
	}

	std::string printTime(const std::chrono::nanoseconds& duration)
	{
		struct Bound
		{
			const char* _units;
			std::chrono::nanoseconds::rep _scale;
			std::chrono::nanoseconds::rep _maximum;
		};

		static const std::array<Bound, 10> bounds{
			Bound{ "ns", 1, 999 },
			Bound{ "us", 100, 9'999 },
			Bound{ "us", 10, 99'999 },
			Bound{ "us", 1, 999'999 },
			Bound{ "ms", 100, 9'999'999 },
			Bound{ "ms", 10, 99'999'999 },
			Bound{ "ms", 1, 999'999'999 },
			Bound{ "s", 100, 9'999'999'999 },
			Bound{ "s", 10, 99'999'999'999 },
			Bound{ "s", 1, std::numeric_limits<std::chrono::nanoseconds::rep>::max() },
		};

		const auto nanoseconds = duration.count();
		const auto i = std::find_if_not(bounds.begin(), bounds.end(), [nanoseconds](const Bound& bound) { return nanoseconds > bound._maximum; });
		assert(i != bounds.end());
		const auto scale = (i->_maximum + 1) / 1'000;
		const auto value = (nanoseconds + scale - 1) / scale;
		const auto whole = value / i->_scale;
		const auto fraction = value % i->_scale;
		return (fraction ? std::to_string(whole) + '.' + std::to_string(fraction) : std::to_string(whole)) + i->_units;
	}

	struct Measurement
	{
		using Duration = std::chrono::high_resolution_clock::duration;

		Duration::rep _iterations = 0;
		Duration _totalDuration{ 0 };
		Duration _minDuration = Duration::max();
		Duration _maxDuration{ 0 };

		auto average() const noexcept { return Duration{ (_totalDuration.count() + _iterations - 1) / _iterations }; }
	};

	template <Measurement::Duration::rep maxIterations = std::numeric_limits<Measurement::Duration::rep>::max(), typename Payload, typename Cleanup>
	auto measure(Payload&& payload, Cleanup&& cleanup, const std::chrono::seconds& minDuration = std::chrono::seconds{ 1 })
	{
		Measurement measurement;
		for (;;)
		{
			const auto startTime = std::chrono::high_resolution_clock::now();
			payload();
			const auto duration = std::chrono::high_resolution_clock::now() - startTime;
			++measurement._iterations;
			measurement._totalDuration += duration;
			if (measurement._minDuration > duration)
				measurement._minDuration = duration;
			if (measurement._maxDuration < duration)
				measurement._maxDuration = duration;
			if (measurement._iterations >= maxIterations || measurement._totalDuration >= minDuration)
				break;
			cleanup();
		}
		return measurement;
	}
}

int main(int argc, char** argv)
{
	std::filesystem::path path;
	if (int i = 1; i < argc)
		path = argv[i];
	else
	{
		std::cerr << "No input file specified\n";
		return 1;
	}

	const auto [data, size] = ::load(path);
	if (!data)
		return 1;

	std::unique_ptr<seir::synth::Composition> composition;
	const auto parsing = ::measure<10'000>(
		[&composition, source = data.get()] { composition = seir::synth::Composition::create(source); }, // Clang 13 is unable to capture 'data' by reference.
		[&composition] { composition.reset(); });

	static constexpr seir::synth::AudioFormat format{ 48'000, seir::synth::ChannelLayout::Stereo };
	std::unique_ptr<seir::synth::Renderer> renderer;
	const auto preparation = ::measure<10'000>(
		[&renderer, &composition] { renderer = seir::synth::Renderer::create(*composition, format); },
		[&renderer] { renderer.reset(); });

	static constexpr size_t bufferFrames = format.samplingRate();
	const auto buffer = std::make_unique<float[]>(bufferFrames * format.bytesPerFrame());

	size_t compositionFrames = 0;
	for (;;)
	{
		const auto framesRendered = renderer->render(buffer.get(), bufferFrames);
		if (!framesRendered)
			break;
		compositionFrames += framesRendered;
	}
	renderer->restart();

	const auto compositionDuration = static_cast<double>(compositionFrames) * double{ Measurement::Duration::period::den } / format.samplingRate();

	const auto baseline = ::measure(
		[&buffer, compositionFrames] {
			for (auto remainingFrames = compositionFrames; remainingFrames > 0;)
			{
				const auto iterationFrames = std::min(remainingFrames, bufferFrames);
				std::memset(buffer.get(), static_cast<int>(remainingFrames / bufferFrames), iterationFrames * 2 * sizeof(float));
				remainingFrames -= iterationFrames;
			}
		},
		[] {},
		std::chrono::seconds{ 5 });
	const auto rendering = ::measure(
		[&renderer, bufferData = buffer.get()] { while (renderer->render(bufferData, bufferFrames) > 0) ; },
		[&renderer] { renderer->restart(); },
		std::chrono::seconds{ 5 });

	std::cout << "ParseTime: " << ::printTime(parsing.average()) << " [N=" << parsing._iterations << ", min=" << ::printTime(parsing._minDuration) << ", max=" << ::printTime(parsing._maxDuration) << "]\n";
	std::cout << "PrepareTime: " << ::printTime(preparation.average()) << " [N=" << preparation._iterations << ", min=" << ::printTime(preparation._minDuration) << ", max=" << ::printTime(preparation._maxDuration) << "]\n";
	std::cout << "RenderTime: " << ::printTime(rendering.average()) << " [N=" << rendering._iterations << ", min=" << ::printTime(rendering._minDuration) << ", max=" << ::printTime(rendering._maxDuration) << "]\n";
	std::cout << "RenderSpeed: " << compositionDuration / static_cast<double>(rendering.average().count()) << "x ("
			  << std::to_string(static_cast<double>(compositionFrames * 2 * sizeof(float)) * std::ldexp(1'000'000'000, -20) / static_cast<double>(rendering.average().count())) << " MiB/s, "
			  << std::to_string(static_cast<double>(compositionFrames * 2 * sizeof(float)) * 8. / static_cast<double>(rendering.average().count())) << " Gbit/s, "
			  << std::to_string(static_cast<double>(rendering.average().count()) / static_cast<double>(baseline.average().count())) << " memsets)\n";
	return 0;
}
