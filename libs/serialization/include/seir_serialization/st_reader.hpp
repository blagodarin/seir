// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <memory>
#include <string_view>

namespace seir
{
	class Blob;
	template <class>
	class SharedPtr;

	class StToken
	{
	public:
		enum class Type
		{
			Name,
			Value,
			ListBegin,
			ListEnd,
			ObjectBegin,
			ObjectEnd,
			End,
			Error,
		};

		constexpr StToken(size_t line, ptrdiff_t column, Type type, std::string_view text) noexcept
			: _line{ line }, _column{ column }, _type{ type }, _text{ text } {}

		[[nodiscard]] constexpr ptrdiff_t column() const noexcept { return _column; }
		[[nodiscard]] constexpr size_t line() const noexcept { return _line; }
		[[nodiscard]] constexpr std::string_view text() const noexcept { return _text; }
		[[nodiscard]] constexpr Type type() const noexcept { return _type; }

	private:
		size_t _line;
		ptrdiff_t _column;
		Type _type;
		std::string_view _text;
	};

	class StReader
	{
	public:
		explicit StReader(const SharedPtr<Blob>&);
		~StReader() noexcept;

		[[nodiscard]] StToken read();

	private:
		const std::unique_ptr<class StReaderImpl> _impl;
	};
}
