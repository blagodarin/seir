// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/endian.hpp>

namespace seir
{
	enum : uint16_t
	{
		WAVE_FORMAT_PCM = 0x0001,
		WAVE_FORMAT_IEEE_FLOAT = 0x0003,
	};

#pragma pack(push, 1)

	struct WavFileHeader
	{
		enum : uint32_t
		{
			RIFF = makeCC('R', 'I', 'F', 'F'),
			WAVE = makeCC('W', 'A', 'V', 'E'),
		};

		uint32_t _riffId;
		uint32_t _riffSize;
		uint32_t _waveId;
	};

	struct WavChunkHeader
	{
		enum : uint32_t
		{
			fmt = makeCC('f', 'm', 't', ' '),
			data = makeCC('d', 'a', 't', 'a'),
		};

		uint32_t _id;
		uint32_t _size;
	};

	struct WavFormatChunk
	{
		uint16_t format;
		uint16_t channels;
		uint32_t samplesPerSecond;
		uint32_t bytesPerSecond;
		uint16_t blockAlign;
		uint16_t bitsPerSample;
	};

#pragma pack(pop)
}
