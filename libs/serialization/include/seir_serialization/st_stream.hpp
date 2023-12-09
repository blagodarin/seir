// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_serialization/st_reader.hpp>

#include <stdexcept>

namespace seir
{
	class StStreamError : public std::runtime_error
	{
	public:
		explicit StStreamError(const StToken&);
		[[nodiscard]] size_t column() const noexcept { return _column; }
		[[nodiscard]] size_t line() const noexcept { return _line; }

	private:
		const size_t _line;
		const size_t _column;
	};

	class StStream
	{
	public:
		explicit StStream(StReader& reader)
			: _reader{ reader }, _token{ _reader.read() } {}

		void beginList() { next(StToken::Type::ListBegin); }
		void beginObject() { next(StToken::Type::ObjectBegin); }
		void endList() { next(StToken::Type::ListEnd); }
		void endObject() { next(StToken::Type::ObjectEnd); }
		void name(std::string_view expected) { nextText(StToken::Type::Name, expected); }
		[[nodiscard]] std::string_view name() { return nextText(StToken::Type::Name); }
		[[nodiscard]] bool tryBeginList() { return tryNext(StToken::Type::ListBegin); }
		[[nodiscard]] bool tryBeginObject() { return tryNext(StToken::Type::ObjectBegin); }
		[[nodiscard]] bool tryEnd() const noexcept { return _token.type() == StToken::Type::End; }
		[[nodiscard]] bool tryEndList() { return tryNext(StToken::Type::ListEnd); }
		[[nodiscard]] bool tryEndObject() { return tryNext(StToken::Type::ObjectEnd); }
		[[nodiscard]] bool tryName(std::string_view expected) { return tryNextText(StToken::Type::Name, expected); }
		void value(std::string_view expected) { nextText(StToken::Type::Value, expected); }
		[[nodiscard]] std::string_view value() { return nextText(StToken::Type::Value); }

	private:
		void next(StToken::Type);
		void nextText(StToken::Type, std::string_view);
		std::string_view nextText(StToken::Type);
		bool tryNext(StToken::Type);
		bool tryNextText(StToken::Type, std::string_view);

	private:
		StReader& _reader;
		StToken _token;
	};
}
