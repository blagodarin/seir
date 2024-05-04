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
		IsRoot = 1 << 0,
		IsNonEmptyObject = 1 << 1,
		EndsWithKey = 1 << 2, // Implies IsNonEmptyObject.
		IsList = 1 << 3,
	};
}

namespace seir
{
	StWriter::StWriter(std::string& buffer, Formatting formatting)
		: _buffer{ buffer }
		, _stack{ IsRoot }
		, _pretty{ formatting == StWriter::Formatting::Pretty }
	{
	}

	StWriter::~StWriter() noexcept = default;

	void StWriter::addKey(std::string_view key)
	{
		// TODO: Add key validation.
		const auto entry = _stack.back();
		if (entry & IsList) [[unlikely]]
			throw StWriter::UnexpectedToken{};
		if (_pretty)
		{
			if (!_buffer.empty())
				_buffer += '\n';
			_buffer.append(2 * (_stack.size() - 1), ' ');
		}
		else if (entry & EndsWithKey)
			_buffer += ' ';
		_buffer += key;
		_stack.back() = static_cast<uint8_t>(entry | IsNonEmptyObject | EndsWithKey);
	}

	void StWriter::addValue(std::string_view value)
	{
		const auto entry = _stack.back();
		if (!(entry & (IsNonEmptyObject | IsList))) [[unlikely]]
			throw StWriter::UnexpectedToken{};
		if (_pretty)
			beginPrettyValue(entry);
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
		_stack.back() = static_cast<uint8_t>(entry & ~EndsWithKey);
	}

	void StWriter::beginList()
	{
		const auto entry = _stack.back();
		if (!(entry & (IsNonEmptyObject | IsList))) [[unlikely]]
			throw StWriter::UnexpectedToken{};
		if (_pretty)
			beginPrettyValue(entry);
		_buffer += '[';
		_stack.back() = static_cast<uint8_t>(entry & ~EndsWithKey);
		_stack.emplace_back(IsList);
	}

	void StWriter::beginObject()
	{
		const auto entry = _stack.back();
		if (!(entry & (IsNonEmptyObject | IsList))) [[unlikely]]
			throw StWriter::UnexpectedToken{};
		if (_pretty)
			beginPrettyValue(entry);
		_buffer += '{';
		_stack.back() = static_cast<uint8_t>(entry & ~EndsWithKey);
		_stack.emplace_back(uint8_t{});
	}

	void StWriter::endList()
	{
		if (!(_stack.back() & IsList)) [[unlikely]]
			throw StWriter::UnexpectedToken{};
		if (_pretty)
		{
			_buffer += '\n';
			_buffer.append(2 * (_stack.size() - 2), ' ');
		}
		_buffer += ']';
		_stack.pop_back();
		assert(!_stack.empty());
	}

	void StWriter::endObject()
	{
		if (_stack.back() & (IsRoot | IsList)) [[unlikely]]
			throw StWriter::UnexpectedToken{};
		if (_pretty)
		{
			_buffer += '\n';
			_buffer.append(2 * (_stack.size() - 2), ' ');
		}
		_buffer += '}';
		_stack.pop_back();
		assert(!_stack.empty());
	}

	void StWriter::finish()
	{
		if (_stack.size() != 1) [[unlikely]]
			throw StWriter::UnexpectedToken{};
		assert((_stack.back() & (IsRoot | IsList)) == IsRoot);
		if (_pretty && _stack.back() & IsNonEmptyObject)
			_buffer += '\n';
	}

	void StWriter::beginPrettyValue(uint8_t entry)
	{
		if (entry & IsList)
		{
			_buffer += '\n';
			_buffer.append(2 * (_stack.size() - 1), ' ');
		}
		else
			_buffer += ' ';
	}
}
