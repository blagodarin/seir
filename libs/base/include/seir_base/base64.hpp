// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstdint>
#include <span>

namespace seir
{
	//
	[[nodiscard]] constexpr size_t base64EncodedSize(size_t size) noexcept;

	// Encodes data using base64url encoding (RFC 4648, see https://datatracker.ietf.org/doc/html/rfc4648).
	[[nodiscard]] constexpr bool encodeBase64Url(std::span<char> output, std::span<const std::byte> input) noexcept;

	//
	[[nodiscard]] constexpr size_t base64DecodedSize(size_t size) noexcept;

	// Decodes base64url-encoded data (RFC 4648, see https://datatracker.ietf.org/doc/html/rfc4648).
	[[nodiscard]] constexpr bool decodeBase64Url(std::span<std::byte> output, std::span<const char> input) noexcept;
}

constexpr size_t seir::base64EncodedSize(size_t size) noexcept
{
	return size + (size + 2) / 3;
}

constexpr bool seir::encodeBase64Url(std::span<char> output, std::span<const std::byte> input) noexcept
{
	constexpr char table[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

	if (output.size() < base64EncodedSize(input.size()))
		return false;
	auto out = output.data();
	auto in = input.data();
	const auto tail = input.size() % 3;
	for (const auto end = in + input.size() - tail; in != end;)
	{
		auto value = std::to_integer<uint32_t>(*in++) << 16;
		*out++ = table[value >> 18];
		value += std::to_integer<uint32_t>(*in++) << 8;
		*out++ = table[(value >> 12) & 0b111111];
		value += std::to_integer<uint32_t>(*in++);
		*out++ = table[(value >> 6) & 0b111111];
		*out++ = table[value & 0b111111];
	}
	if (tail > 0)
	{
		auto value = std::to_integer<uint32_t>(in[0]) << 16;
		out[0] = table[value >> 18];
		if (tail > 1)
		{
			value += std::to_integer<uint32_t>(in[1]) << 8;
			out[2] = table[(value >> 6) & 0b111111];
		}
		out[1] = table[(value >> 12) & 0b111111];
	}
	return true;
}

constexpr size_t seir::base64DecodedSize(size_t size) noexcept
{
	return size - (size >> 2) - static_cast<size_t>((size & 0b11) > 0);
}

constexpr bool seir::decodeBase64Url(std::span<std::byte> output, std::span<const char> input) noexcept
{
	constexpr uint8_t kBad = 0xFF;
	constexpr uint8_t table[256]{
		kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad,
		kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad,
		kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, 0x3E, kBad, kBad, //   ! " # $ % & ' ( ) * + , - . /
		0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, kBad, kBad, kBad, kBad, kBad, kBad, // 0 1 2 3 4 5 6 7 8 9 : ; < = > ?
		kBad, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, // @ A B C D E F G H I J K L M N O
		0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, kBad, kBad, kBad, kBad, 0x3F, // P Q R S T U V W X Y Z [ \ ] ^ _
		kBad, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, // ` a b c d e f g h i j k l m n o
		0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, kBad, kBad, kBad, kBad, kBad, // p q r s t u v w x y z { | } ~
		kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad,
		kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad,
		kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad,
		kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad,
		kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad,
		kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad,
		kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad,
		kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad, kBad
	};

	const auto tail = input.size() & 0b11;
	if (tail == 1 || output.size() < input.size() - (input.size() >> 2) - static_cast<size_t>(tail > 0))
		return false;
	auto out = output.data();
	auto in = input.data();
	for (const auto end = in + input.size() - tail; in != end;)
	{
		uint32_t value = 0;
		for (size_t i = 0; i < 4; ++i)
		{
			if (const auto next = table[static_cast<uint8_t>(*in++)]; next == kBad)
				return false;
			else
				value = (value << 6) + next;
		}
		*out++ = static_cast<std::byte>(value >> 16);
		*out++ = static_cast<std::byte>((value >> 8) & 0xFF);
		*out++ = static_cast<std::byte>(value & 0xFF);
	}
	if (tail > 0)
	{
		uint32_t value = 0;
		for (size_t i = 0; i < tail; ++i)
		{
			if (const auto next = table[static_cast<uint8_t>(*in++)]; next == kBad)
				return false;
			else
				value = (value << 6) + next;
		}
		value <<= (4 - tail) * 6;
		out[0] = static_cast<std::byte>(value >> 16);
		if (tail == 3)
			out[1] = static_cast<std::byte>((value >> 8) & 0xFF);
	}
	return true;
}
