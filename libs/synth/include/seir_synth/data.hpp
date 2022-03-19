// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_synth/common.hpp>

#include <map>
#include <memory>
#include <string>

namespace seir::synth
{
	class Composition;

	struct SequenceData
	{
		std::vector<Sound> _sounds;
	};

	struct TrackData
	{
		std::shared_ptr<TrackProperties> _properties;
		std::vector<std::shared_ptr<SequenceData>> _sequences;
		std::map<size_t, std::shared_ptr<SequenceData>> _fragments;

		explicit TrackData(std::shared_ptr<TrackProperties>&& properties) noexcept
			: _properties{ std::move(properties) } {}
	};

	struct PartData
	{
		std::shared_ptr<VoiceData> _voice;
		std::string _voiceName;
		std::vector<std::shared_ptr<TrackData>> _tracks;

		explicit PartData(const std::shared_ptr<VoiceData>& voice) noexcept
			: _voice{ voice } {}
	};

	// Contains composition data in an editable format.
	struct CompositionData
	{
		unsigned _speed = kMinSpeed;
		unsigned _loopOffset = 0;
		unsigned _loopLength = 0;
		float _gainDivisor = 1;
		std::vector<std::shared_ptr<PartData>> _parts;
		std::string _title;
		std::string _author;

		CompositionData() = default;
		CompositionData(const Composition&);
		CompositionData(const std::shared_ptr<VoiceData>&, Note);

		[[nodiscard]] std::unique_ptr<Composition> pack() const;
	};

	std::vector<std::byte> serialize(const Composition&);
}
