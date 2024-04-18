// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_serialization/st_writer.hpp>

#include <algorithm>
#include <array>
#include <cassert>

namespace
{
	enum : uint8_t
	{
		IsObject = 1 << 0,
		AcceptsValues = 1 << 1,
		HasValues = 1 << 2,
		IsRoot = 1 << 3,
	};
}

namespace seir
{
	StWriter::StWriter(Formatting formatting)
		: _stack{ IsObject | HasValues | IsRoot }
		, _pretty{ formatting == StWriter::Formatting::Pretty }
	{
	}

	StWriter::~StWriter() noexcept = default;

	void StWriter::addKey(std::string_view key)
	{
		// TODO: Add key validation.
		const auto entry = _stack.back();
		if (!(entry & IsObject)) [[unlikely]]
			throw StWriter::UnexpectedToken{};
		if (_pretty)
		{
			if (!_buffer.empty())
				_buffer += '\n';
			_buffer.append(_stack.size() - 1, '\t');
		}
		else if (!(entry & HasValues))
			_buffer += ' ';
		_buffer += key;
		_stack.back() = static_cast<uint8_t>((entry | AcceptsValues) & ~HasValues);
	}

	void StWriter::addValue(std::string_view value)
	{
		const auto entry = _stack.back();
		if (!(entry & AcceptsValues)) [[unlikely]]
			throw StWriter::UnexpectedToken{};
		if (_pretty)
		{
			if (entry & IsObject)
				_buffer += ' ';
			else
			{
				_buffer += '\n';
				_buffer.append(_stack.size() - 1, '\t');
			}
		}
		_buffer += '"';
		for (auto begin = value.data(), end = begin + value.size();;)
		{
			const auto i = std::find_if(begin, end, [](char c) { return c == '\\' || c == '"'; });
			if (i != begin)
				_buffer.append(begin, static_cast<size_t>(i - begin));
			if (i == end)
				break;
			const std::array<char, 2> sequence{ '\\', *i };
			_buffer += std::string_view{ sequence.data(), sequence.size() };
			begin = i + 1;
		}
		_buffer += '"';
		_stack.back() = static_cast<uint8_t>(entry | HasValues);
	}

	void StWriter::beginList()
	{
		const auto entry = _stack.back();
		if (!(entry & AcceptsValues)) [[unlikely]]
			throw StWriter::UnexpectedToken{};
		if (_pretty)
			_buffer += ' ';
		_buffer += '[';
		_stack.back() = static_cast<uint8_t>(entry | HasValues);
		_stack.emplace_back(AcceptsValues);
	}

	void StWriter::beginObject()
	{
		const auto entry = _stack.back();
		if (!(entry & AcceptsValues)) [[unlikely]]
			throw StWriter::UnexpectedToken{};
		if (_pretty)
			_buffer += ' ';
		_buffer += '{';
		_stack.back() = static_cast<uint8_t>(entry | HasValues);
		_stack.emplace_back(static_cast<uint8_t>(IsObject | HasValues));
	}

	void StWriter::endList()
	{
		if (_stack.back() & IsObject) [[unlikely]]
			throw StWriter::UnexpectedToken{};
		if (_pretty)
		{
			_buffer += '\n';
			_buffer.append(_stack.size() - 2, '\t');
		}
		_buffer += ']';
		_stack.pop_back();
		assert(!_stack.empty());
	}

	void StWriter::endObject()
	{
		if ((_stack.back() & (IsObject | IsRoot)) != IsObject) [[unlikely]]
			throw StWriter::UnexpectedToken{};
		if (_pretty)
		{
			_buffer += '\n';
			_buffer.append(_stack.size() - 2, '\t');
		}
		_buffer += '}';
		_stack.pop_back();
		assert(!_stack.empty());
	}

	std::string StWriter::commit(StWriter&& writer)
	{
		if (writer._stack.size() != 1) [[unlikely]]
			throw StWriter::UnexpectedToken{};
		if (writer._pretty && !writer._buffer.empty())
			writer._buffer += '\n';
		return std::move(writer._buffer);
	}
}
