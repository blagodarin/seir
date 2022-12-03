// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstdint>
#include <span>

namespace seir
{
	//
	[[nodiscard]] constexpr size_t base85EncodedSize(size_t size) noexcept;

	// Encodes data using Z85 encoding (see https://rfc.zeromq.org/spec/32/).
	[[nodiscard]] constexpr bool encodeZ85(std::span<char> output, std::span<const std::byte> input) noexcept;

	//
	[[nodiscard]] constexpr size_t base85DecodedSize(size_t size) noexcept;

	// Decodes Z85-encoded data (see https://rfc.zeromq.org/spec/32/).
	[[nodiscard]] constexpr bool decodeZ85(std::span<std::byte> output, std::span<const char> input) noexcept;
}

constexpr size_t seir::base85EncodedSize(size_t size) noexcept
{
	return size + ((size + 3) >> 2);
}

constexpr bool seir::encodeZ85(std::span<char> output, std::span<const std::byte> input) noexcept
{
	constexpr char table[86] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.-:+=^!/*?&<>()[]{}@%$#";

	if (output.size() < base85EncodedSize(input.size()))
		return false;
	auto out = output.data();
	auto in = input.data();
	const auto tail = input.size() & 0b11;
	for (const auto end = in + input.size() - tail; in != end;)
	{
		auto value = std::to_integer<uint32_t>(*in++) << 24;
		value += std::to_integer<uint32_t>(*in++) << 16;
		value += std::to_integer<uint32_t>(*in++) << 8;
		value += std::to_integer<uint32_t>(*in++);
		*out++ = table[value / (85 * 85 * 85 * 85)];
		*out++ = table[value / (85 * 85 * 85) % 85];
		*out++ = table[value / (85 * 85) % 85];
		*out++ = table[value / 85 % 85];
		*out++ = table[value % 85];
	}
	if (tail > 0)
	{
		auto value = std::to_integer<uint32_t>(in[0]) << 24;
		if (tail > 1)
		{
			value += std::to_integer<uint32_t>(in[1]) << 16;
			if (tail > 2)
			{
				value += std::to_integer<uint32_t>(in[2]) << 8;
				out[3] = table[(value + 84) / 85 % 85];
				out[2] = table[value / (85 * 85) % 85];
			}
			else
				out[2] = table[(value + 84 * 85) / (85 * 85) % 85];
			out[1] = table[value / (85 * 85 * 85) % 85];
		}
		else
			out[1] = table[(value + 84 * 85 * 85) / (85 * 85 * 85) % 85];
		out[0] = table[value / (85 * 85 * 85 * 85)]; // cppcheck-suppress[unreadVariable]
	}
	return true;
}

constexpr size_t seir::base85DecodedSize(size_t size) noexcept
{
	return size - size / 5 - static_cast<size_t>(size % 5 > 0);
}

constexpr bool seir::decodeZ85(std::span<std::byte> output, std::span<const char> input) noexcept
{
	constexpr auto kBad = 0xFF;
	constexpr uint8_t table[256]{
		kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad,
		kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad,
		kBad, 0x44, kBad, 0x54, 0x53, 0x52, 0x48, kBad, 0x4B, 0x4C, 0x46, 0x41, kBad, 0x3F, 0x3E, 0x45, //   ! " # $ % & ' ( ) * + , - . /
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x40, kBad, 0x49, 0x42, 0x4A, 0x47, // 0 1 2 3 4 5 6 7 8 9 : ; < = > ?
		0x51, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, // @ A B C D E F G H I J K L M N O
		0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x4D, kBad, 0x4E, 0x43, kBad, // P Q R S T U V W X Y Z [ \ ] ^ _
		kBad, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, // ` a b c d e f g h i j k l m n o
		0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x4F, kBad, 0x50, kBad, kBad, // p q r s t u v w x y z { | } ~
		kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad,
		kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad,
		kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad,
		kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad,
		kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad,
		kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad,
		kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad,
		kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad
	};

	const auto tail = input.size() % 5;
	if (tail == 1 || output.size() < input.size() - input.size() / 5 - static_cast<size_t>(tail > 0))
		return false;
	auto out = output.data();
	auto in = input.data();
	for (const auto end = in + input.size() - tail; in != end;)
	{
		uint64_t value = 0;
		for (size_t i = 0; i < 5; ++i)
		{
			if (const auto next = table[static_cast<uint8_t>(*in++)]; next == kBad)
				return false;
			else
				value = value * 85 + next;
		}
		if (value > 0xFFFFFFFFu)
			return false;
		*out++ = static_cast<std::byte>(value >> 24);
		*out++ = static_cast<std::byte>((value >> 16) & 0xFF);
		*out++ = static_cast<std::byte>((value >> 8) & 0xFF);
		*out++ = static_cast<std::byte>(value & 0xFF);
	}
	if (tail > 0)
	{
		uint64_t value = 0;
		for (size_t i = 0; i < tail; ++i)
		{
			if (const auto next = table[static_cast<uint8_t>(*in++)]; next == kBad)
				return false;
			else
				value = value * 85 + next;
		}
		for (auto i = tail; i < 5; ++i)
			value *= 85;
		switch (tail)
		{
		case 4: out[2] = static_cast<std::byte>((value >> 8) & 0xFF); [[fallthrough]];
		case 3: out[1] = static_cast<std::byte>((value >> 16) & 0xFF); [[fallthrough]];
		case 2: out[0] = static_cast<std::byte>(value >> 24); // cppcheck-suppress[unreadVariable]
		}
	}
	return true;
}
