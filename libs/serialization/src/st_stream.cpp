// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_serialization/st_stream.hpp>

namespace seir
{
	StStreamError::StStreamError(const StToken& token)
		: std::runtime_error{ "StStreamError" }
		, _line{ token.line() }
		, _column{ token.column() }
	{
	}

	void StStream::next(StToken::Type type)
	{
		if (_token.type() != type)
			throw StStreamError{ _token };
		_token = _reader.read();
	}

	void StStream::nextText(StToken::Type type, std::string_view text)
	{
		if (_token.type() != type || _token.text() != text)
			throw StStreamError{ _token };
		_token = _reader.read();
	}

	std::string_view StStream::nextText(StToken::Type type)
	{
		if (_token.type() != type)
			throw StStreamError{ _token };
		auto result = _token.text();
		_token = _reader.read();
		return result;
	}

	bool StStream::tryNext(StToken::Type type)
	{
		if (_token.type() != type)
			return false;
		_token = _reader.read();
		return true;
	}

	bool StStream::tryNextText(StToken::Type type, std::string_view text)
	{
		if (_token.type() != type || _token.text() != text)
			return false;
		_token = _reader.read();
		return true;
	}

	std::optional<std::string_view> StStream::tryNextText(StToken::Type type)
	{
		if (_token.type() != type)
			return {};
		auto result = _token.text();
		_token = _reader.read();
		return result;
	}
}
