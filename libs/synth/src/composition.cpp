// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "composition.hpp"

#include <seir_base/int_utils.hpp>
#include <seir_synth/shaper.hpp>

#include <cassert>
#include <charconv>
#include <limits>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <string>

#include <fmt/format.h>

namespace
{
	struct Location
	{
		size_t _line = 1;
		ptrdiff_t _offset = 1;
	};

	class CompositionError : public std::runtime_error
	{
	public:
		CompositionError(const Location& location, const std::string& message)
			: std::runtime_error{ fmt::format("({}:{}) {}", location._line, location._offset + 1, message) } {}
	};
}

namespace seir::synth
{
	void CompositionImpl::load(const char* source)
	{
		enum class Section
		{
			Global,
			Voice,
			Track,
			Sequences,
			Fragments,
		};

		size_t line = 1;
		const char* lineBase = source;
		Section currentSection = Section::Global;
		VoiceData* currentVoice = nullptr;
		TrackProperties* currentTrackProperties = nullptr;

		const auto location = [&]() -> Location { return { line, source - lineBase }; };

		const auto skipSpaces = [&] {
			if (*source != ' ' && *source != '\t' && *source != '\n' && *source != '\r' && *source)
				throw CompositionError{ location(), "Space expected" };
			while (*source == ' ' || *source == '\t')
				++source;
		};

		const auto consumeEndOfLine = [&] {
			if (*source == '\r')
			{
				++source;
				if (*source == '\n')
					++source;
			}
			else if (*source == '\n')
				++source;
			else
			{
				if (*source)
					throw CompositionError{ location(), "End of line expected" };
				return;
			}
			++line;
			lineBase = source;
		};

		const auto tryReadIdentifier = [&] {
			if (!((*source >= 'a' && *source <= 'z') || *source == '_'))
				return std::string_view{};
			const auto begin = source;
			do
				++source;
			while ((*source >= 'a' && *source <= 'z') || (*source >= '0' && *source <= '9') || *source == '_');
			const std::string_view result{ begin, static_cast<size_t>(source - begin) };
			skipSpaces();
			return result;
		};

		const auto readIdentifier = [&] {
			const auto result = tryReadIdentifier();
			if (result.empty())
				throw CompositionError{ location(), "Identifier expected" };
			return result;
		};

		const auto tryReadInt = [&](int min, int max) -> std::optional<int> {
			const auto begin = source;
			const auto isNegative = *source == '-';
			if (isNegative)
				++source;
			if (*source < '0' || *source > '9')
				return {};
			do
				++source;
			while (*source >= '0' && *source <= '9');
			int result; // NOLINT(cppcoreguidelines-init-variables)
			if (std::from_chars(begin, source, result).ec != std::errc{})
				throw CompositionError{ { line, begin - lineBase }, "Number expected" };
			if (result < min || result > max)
				throw CompositionError{ { line, begin - lineBase }, "Number is out of range" };
			skipSpaces();
			return result;
		};

		const auto readInt = [&](int min, int max) {
			const auto result = tryReadInt(min, max);
			if (!result)
				throw CompositionError{ location(), "Number expected" };
			return *result;
		};

		const auto tryReadUnsigned = [&](unsigned min, unsigned max, bool needSpace = true) -> std::optional<unsigned> {
			if (*source < '0' || *source > '9')
				return {};
			const auto begin = source;
			do
				++source;
			while (*source >= '0' && *source <= '9');
			unsigned result; // NOLINT(cppcoreguidelines-init-variables)
			if (std::from_chars(begin, source, result).ec != std::errc{})
				throw CompositionError{ { line, begin - lineBase }, "Number expected" };
			if (result < min || result > max)
				throw CompositionError{ { line, begin - lineBase }, "Number is out of range" };
			if (needSpace)
				skipSpaces();
			return result;
		};

		const auto readUnsigned = [&](unsigned min, unsigned max, bool needSpace = true) {
			const auto result = tryReadUnsigned(min, max, needSpace);
			if (!result)
				throw CompositionError{ location(), "Number expected" };
			return *result;
		};

		const auto tryReadFloat = [&](float min, float max) -> std::optional<float> {
			if (!(*source >= '0' && *source <= '9') && *source != '-')
				return {};
			const auto begin = source;
			do
				++source;
			while (*source >= '0' && *source <= '9');
			if (*source == '.')
				do
					++source;
				while (*source >= '0' && *source <= '9');
#ifdef _MSC_VER
			float result;
			if (std::from_chars(begin, source, result).ec != std::errc{})
				throw CompositionError{ { line, begin - lineBase }, "Bad number" };
#else
			const auto result = std::strtof(std::string{ begin, source }.c_str(), nullptr);
#endif
			if (result < min || result > max)
				throw CompositionError{ { line, begin - lineBase }, "Number is out of range" };
			skipSpaces();
			return result;
		};

		const auto readFloat = [&](float min, float max) {
			const auto result = tryReadFloat(min, max);
			if (!result)
				throw CompositionError{ location(), "Number expected" };
			return *result;
		};

		const auto tryReadString = [&]() -> std::optional<std::string> {
			if (*source != '"')
				return {};
			const auto begin = ++source;
			while (*source && *source != '"')
				++source;
			if (!*source)
				throw CompositionError{ { line, begin - lineBase }, "Unexpected end of file" };
			const auto end = source++;
			skipSpaces();
			return std::string{ begin, end };
		};

		const auto readString = [&] {
			const auto result = tryReadString();
			if (!result)
				throw CompositionError{ location(), "String expected" };
			return *result;
		};

		const auto parseNote = [&](std::vector<Sound>& sequence, size_t delay, size_t baseOffset) {
			assert(*source >= 'A' && *source <= 'G');
			assert(baseOffset < kNotesPerOctave);
			++source;
			if (*source == '#')
			{
				if (baseOffset == kNotesPerOctave - 1)
					throw CompositionError{ location(), "Note overflow" };
				++baseOffset;
				++source;
			}
			else if (*source == 'b')
			{
				if (!baseOffset)
					throw CompositionError{ location(), "Note underflow" };
				--baseOffset;
				++source;
			}
			if (*source < '0' || *source > '8')
				throw CompositionError{ location(), "Bad note" };
			const auto note = static_cast<Note>(toUnsigned(*source - '0') * kNotesPerOctave + baseOffset);
			++source;
			size_t sustain = 0;
			if (*source == '+')
			{
				++source;
				sustain = readUnsigned(0, kMaxSustain, false);
			}
			sequence.emplace_back(delay, note, sustain);
		};

		const auto parseSequence = [&](std::vector<Sound>& sequence) {
			for (size_t delay = 0;;)
			{
				const auto mapNote = [&](char note) -> std::underlying_type_t<seir::synth::Note> {
					switch (note)
					{
					case 'A': return 9;
					case 'B': return 11;
					case 'C': return 0;
					case 'D': return 2;
					case 'E': return 4;
					case 'F': return 5;
					case 'G': return 7;
					default: throw CompositionError{ location(), "Bad note" };
					}
				};

				switch (*source)
				{
				case '\0':
					return;
				case '\r':
				case '\n':
					consumeEndOfLine();
					return;
				case ',':
					++delay;
					++source;
					break;
				default:
					parseNote(sequence, delay, mapNote(*source));
					delay = 0;
					break;
				}
			}
		};

		const auto parseCommand = [&](std::string_view command) {
			const auto readEnvelope = [&](Envelope& envelope, float minValue, float maxValue) {
				envelope._changes.clear();
				while (const auto duration = tryReadUnsigned(0, static_cast<unsigned>(EnvelopeChange::kMaxDuration.count())))
					envelope._changes.emplace_back(std::chrono::milliseconds{ *duration }, readFloat(minValue, maxValue));
				if (const auto option = tryReadIdentifier(); !option.empty())
				{
					if (option != "sustain")
						throw CompositionError{ location(), "Bad envelope option" };
					envelope._sustainIndex = readUnsigned(0, static_cast<unsigned>(envelope._changes.size()));
				}
				else
					envelope._sustainIndex = 0;
			};

			const auto readOscillation = [&](Oscillation& oscillation) {
				oscillation._frequency = readFloat(1.f, 127.f);
				oscillation._magnitude = readFloat(0.f, 1.f);
			};

			if (command == "amplitude")
			{
				if (currentSection != Section::Voice)
					throw CompositionError{ location(), "Unexpected command" };
				readEnvelope(currentVoice->_amplitudeEnvelope, 0.f, 1.f);
			}
			else if (command == "asymmetry")
			{
				if (currentSection != Section::Voice)
					throw CompositionError{ location(), "Unexpected command" };
				readEnvelope(currentVoice->_asymmetryEnvelope, 0.f, 1.f);
			}
			else if (command == "asymmetry_osc")
			{
				if (currentSection != Section::Voice)
					throw CompositionError{ location(), "Unexpected command" };
				readOscillation(currentVoice->_asymmetryOscillation);
			}
			else if (command == "author")
			{
				if (currentSection != Section::Global)
					throw CompositionError{ location(), "Unexpected command" };
				_author = readString();
			}
			else if (command == "frequency")
			{
				if (currentSection != Section::Voice)
					throw CompositionError{ location(), "Unexpected command" };
				readEnvelope(currentVoice->_frequencyEnvelope, -1.f, 1.f);
			}
			else if (command == "gain")
			{
				if (currentSection != Section::Global)
					throw CompositionError{ location(), "Unexpected command" };
				_gainDivisor = decltype(_gainDivisor)::load(static_cast<uint16_t>(readUnsigned(0, static_cast<unsigned>(std::numeric_limits<uint16_t>::max()))));
			}
			else if (command == "loop")
			{
				if (currentSection != Section::Global)
					throw CompositionError{ location(), "Unexpected command" };
				_loopOffset = readUnsigned(0, std::numeric_limits<unsigned>::max());
				_loopLength = readUnsigned(0, std::numeric_limits<unsigned>::max());
			}
			else if (command == "oscillation")
			{
				if (currentSection != Section::Voice)
					throw CompositionError{ location(), "Unexpected command" };
				readEnvelope(currentVoice->_rectangularityEnvelope, 0.f, 1.f);
			}
			else if (command == "oscillation_osc")
			{
				if (currentSection != Section::Voice)
					throw CompositionError{ location(), "Unexpected command" };
				readOscillation(currentVoice->_rectangularityOscillation);
			}
			else if (command == "polyphony")
			{
				if (currentSection != Section::Track)
					throw CompositionError{ location(), "Unexpected command" };
				if (const auto type = readIdentifier(); type == "chord")
					currentTrackProperties->_polyphony = Polyphony::Chord;
				else if (type == "full")
					currentTrackProperties->_polyphony = Polyphony::Full;
				else
					throw CompositionError{ location(), "Bad polyphony" };
			}
			else if (command == "speed")
			{
				if (currentSection != Section::Global)
					throw CompositionError{ location(), "Unexpected command" };
				_speed = readUnsigned(kMinSpeed, kMaxSpeed);
			}
			else if (command == "stereo_angle")
			{
				if (currentSection != Section::Track)
					throw CompositionError{ location(), "Unexpected command" };
				currentTrackProperties->_sourceOffset = readInt(-90, 90);
			}
			else if (command == "stereo_angular_size")
			{
				if (currentSection != Section::Track)
					throw CompositionError{ location(), "Unexpected command" };
				currentTrackProperties->_sourceWidth = readUnsigned(0, 360);
			}
			else if (command == "stereo_delay")
			{
				if (currentSection != Section::Track)
					throw CompositionError{ location(), "Unexpected command" };
				currentTrackProperties->_headDelay = readFloat(0.f, 1'000.f);
			}
			else if (command == "stereo_distance")
			{
				if (currentSection != Section::Track)
					throw CompositionError{ location(), "Unexpected command" };
				currentTrackProperties->_sourceDistance = readFloat(0.f, 64.f);
			}
			else if (command == "title")
			{
				if (currentSection != Section::Global)
					throw CompositionError{ location(), "Unexpected command" };
				_title = readString();
			}
			else if (command == "tremolo")
			{
				if (currentSection != Section::Voice)
					throw CompositionError{ location(), "Unexpected command" };
				readOscillation(currentVoice->_tremolo);
			}
			else if (command == "vibrato")
			{
				if (currentSection != Section::Voice)
					throw CompositionError{ location(), "Unexpected command" };
				readOscillation(currentVoice->_vibrato);
			}
			else if (command == "wave")
			{
				if (currentSection != Section::Voice)
					throw CompositionError{ location(), "Unexpected command" };
				float minShape = 0;
				float maxShape = 0;
				if (const auto type = readIdentifier(); type == "linear")
					currentVoice->_waveShape = WaveShape::Linear;
				else if (type == "quadratic")
				{
					currentVoice->_waveShape = WaveShape::Quadratic;
					minShape = QuadraticShaper::kMinShape;
					maxShape = QuadraticShaper::kMaxShape;
				}
				else if (type == "quadratic2")
				{
					currentVoice->_waveShape = WaveShape::Quadratic2;
					minShape = Quadratic2Shaper::kMinShape;
					maxShape = Quadratic2Shaper::kMaxShape;
				}
				else if (type == "cubic")
				{
					currentVoice->_waveShape = WaveShape::Cubic;
					minShape = CubicShaper::kMinShape;
					maxShape = CubicShaper::kMaxShape;
				}
				else if (type == "cubic2")
				{
					currentVoice->_waveShape = WaveShape::Cubic2;
					minShape = Cubic2Shaper::kMinShape;
					maxShape = Cubic2Shaper::kMaxShape;
				}
				else if (type == "quintic")
				{
					currentVoice->_waveShape = WaveShape::Quintic;
					minShape = QuinticShaper::kMinShape;
					maxShape = QuinticShaper::kMaxShape;
				}
				else if (type == "cosine")
					currentVoice->_waveShape = WaveShape::Cosine;
				else if (type == "cosine3")
					currentVoice->_waveShape = WaveShape::CosineCubed;
				else
					throw CompositionError{ location(), "Bad voice wave type" };
				if (const auto parameter1 = tryReadFloat(minShape, maxShape); parameter1)
				{
					currentVoice->_waveShapeParameters._shape1 = *parameter1;
					if (const auto parameter2 = tryReadFloat(minShape, maxShape); parameter2)
						currentVoice->_waveShapeParameters._shape2 = *parameter2;
					else
						currentVoice->_waveShapeParameters._shape2 = 0;
				}
				else
					currentVoice->_waveShapeParameters = {};
			}
			else if (command == "weight")
			{
				if (currentSection != Section::Track)
					throw CompositionError{ location(), "Unexpected command" };
				currentTrackProperties->_weight = readUnsigned(1, 255);
			}
			else
				throw CompositionError{ location(), "Unknown command \"" + std::string{ command } + "\"" };
			if (*source && *source != '\n' && *source != '\r')
				throw CompositionError{ location(), "End of line expected" };
		};

		for (;;)
		{
			switch (*source)
			{
			case '\0':
				return;
			case '\r':
			case '\n':
				consumeEndOfLine();
				break;
			case '\t':
			case ' ':
				do
					++source;
				while (*source == ' ' || *source == '\t');
				break;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				if (currentSection == Section::Sequences)
				{
					auto& part = _parts[readUnsigned(1, static_cast<unsigned>(_parts.size())) - 1];
					auto& track = part._tracks[readUnsigned(1, static_cast<unsigned>(part._tracks.size())) - 1];
					const auto sequenceIndex = static_cast<unsigned>(track._sequences.size() + 1);
					readUnsigned(sequenceIndex, sequenceIndex);
					parseSequence(track._sequences.emplace_back());
				}
				else if (currentSection == Section::Fragments)
				{
					auto& part = _parts[readUnsigned(1, static_cast<unsigned>(_parts.size())) - 1];
					auto& track = part._tracks[readUnsigned(1, static_cast<unsigned>(part._tracks.size())) - 1];
					while (const auto delay = tryReadUnsigned(0, std::numeric_limits<unsigned>::max()))
						track._fragments.emplace_back(*delay, readUnsigned(1, static_cast<unsigned>(track._sequences.size())) - 1);
					consumeEndOfLine();
				}
				else
					throw CompositionError{ location(), "Unexpected token" };
				break;
			case '@':
				++source;
				if (const auto section = readIdentifier(); section == "voice")
				{
					const auto partIndex = static_cast<unsigned>(_parts.size() + 1);
					readUnsigned(partIndex, partIndex);
					auto name = tryReadString();
					consumeEndOfLine();
					currentSection = Section::Voice;
					currentVoice = &_parts.emplace_back()._voice;
					if (name)
						_parts.back()._voiceName = std::move(*name);
				}
				else if (section == "track")
				{
					auto& part = _parts[readUnsigned(1, static_cast<unsigned>(_parts.size())) - 1];
					const auto trackIndex = static_cast<unsigned>(part._tracks.size() + 1);
					readUnsigned(trackIndex, trackIndex);
					consumeEndOfLine();
					currentSection = Section::Track;
					currentTrackProperties = &part._tracks.emplace_back()._properties;
				}
				else if (section == "sequences")
				{
					consumeEndOfLine();
					currentSection = Section::Sequences;
				}
				else if (section == "fragments")
				{
					consumeEndOfLine();
					currentSection = Section::Fragments;
				}
				else
					throw CompositionError{ location(), "Unknown section \"@" + std::string{ section } + "\"" };
				break;
			default:
				parseCommand(readIdentifier());
			}
		}
	}

	std::unique_ptr<Composition> Composition::create(const char* textData)
	{
		auto composition = std::make_unique<CompositionImpl>();
		composition->load(textData);
		return composition;
	}
}
