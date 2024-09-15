// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_serialization/st_reader.hpp>

#include <seir_base/buffer.hpp>
#include <seir_io/blob.hpp>

#include <array>
#include <cstring>
#include <vector>

namespace
{
	constexpr uint8_t kAlphanumeric = 0b10000;

	enum CharClass : uint8_t
	{
		Other,
		End,
		Space,
		Cr,
		Lf,
		Quote,
		LBracket,
		RBracket,
		LBrace,
		RBrace,
		Comment,
		Key = kAlphanumeric,
		Digit,
	};

	constexpr std::array<CharClass, 256> kCharClasses{
		End, Other, Other, Other, Other, Other, Other, Other,     // \0
		Other, Space, Lf, Space, Space, Cr, Other, Other,         // \t \n \v \f \r
		Other, Other, Other, Other, Other, Other, Other, Other,   //
		Other, Other, Other, Other, Other, Other, Other, Other,   //
		Space, Other, Quote, Other, Other, Other, Other, Other,   //   ! " # $ % & '
		Other, Other, Other, Other, Other, Other, Other, Comment, // ( ) * + , - . /
		Digit, Digit, Digit, Digit, Digit, Digit, Digit, Digit,   // 0 1 2 3 4 5 6 7
		Digit, Digit, Other, Other, Other, Other, Other, Other,   // 8 9 : ; < = > ?
		Other, Key, Key, Key, Key, Key, Key, Key,                 // @ A B C D E F G
		Key, Key, Key, Key, Key, Key, Key, Key,                   // H I J K L M N O
		Key, Key, Key, Key, Key, Key, Key, Key,                   // P Q R S T U V W
		Key, Key, Key, LBracket, Other, RBracket, Other, Key,     // X Y Z [ \ ] ^ _
		Quote, Key, Key, Key, Key, Key, Key, Key,                 // ` a b c d e f g
		Key, Key, Key, Key, Key, Key, Key, Key,                   // h i j k l m n o
		Key, Key, Key, Key, Key, Key, Key, Key,                   // p q r s t u v w
		Key, Key, Key, LBrace, Other, RBrace, Other, Other,       // x y z { | } ~
		Other, Other, Other, Other, Other, Other, Other, Other,   //
		Other, Other, Other, Other, Other, Other, Other, Other,   //
		Other, Other, Other, Other, Other, Other, Other, Other,   //
		Other, Other, Other, Other, Other, Other, Other, Other,   //
		Other, Other, Other, Other, Other, Other, Other, Other,   //
		Other, Other, Other, Other, Other, Other, Other, Other,   //
		Other, Other, Other, Other, Other, Other, Other, Other,   //
		Other, Other, Other, Other, Other, Other, Other, Other,   //
		Other, Other, Other, Other, Other, Other, Other, Other,   //
		Other, Other, Other, Other, Other, Other, Other, Other,   //
		Other, Other, Other, Other, Other, Other, Other, Other,   //
		Other, Other, Other, Other, Other, Other, Other, Other,   //
		Other, Other, Other, Other, Other, Other, Other, Other,   //
		Other, Other, Other, Other, Other, Other, Other, Other,   //
		Other, Other, Other, Other, Other, Other, Other, Other,   //
		Other, Other, Other, Other, Other, Other, Other, Other,   //
	};

	constexpr CharClass classOf(char c) noexcept
	{
		return ::kCharClasses[static_cast<unsigned char>(c)]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
	}

	template <typename I, typename P>
	constexpr I skipWhile(I iterator, const P& predicate) noexcept
	{
		do
			++iterator;
		while (predicate(*iterator));
		return iterator;
	}
}

namespace seir
{
	class StReaderImpl
	{
	public:
		explicit StReaderImpl(const SharedPtr<Blob>& blob)
			: _size{ blob ? blob->size() : 0 }
		{
			if (blob)
				std::memcpy(_buffer.data(), blob->data(), blob->size());
			_buffer.data()[_size] = std::byte{}; // To simplify parsing.
		}

		StToken read()
		{
			for (;;)
			{
				switch (::classOf(*_cursor))
				{
				case Other:
				case Digit:
					return makeError(_cursor);

				case End:
					if (_cursor == reinterpret_cast<const char*>(_buffer.data()) + _size)
						return makeToken(StToken::Type::End, _cursor, 0);
					else
						return makeError(_cursor);

				case Space:
					_cursor = ::skipWhile(_cursor, [](char c) { return ::classOf(c) == Space; });
					break;

				case Cr:
					if (_cursor[1] == '\n')
						++_cursor;
					[[fallthrough]];
				case Lf:
					_lineBase = _cursor++;
					++_line;
					break;

				case Key:
					if (_stack.back() & AcceptKeys)
					{
						const auto begin = _cursor;
						_cursor = ::skipWhile(_cursor, [](char c) { return ::classOf(c) & kAlphanumeric; });
						_stack.back() |= AcceptValues;
						return makeToken(StToken::Type::Key, begin, _cursor - begin);
					}
					else
						return makeError(_cursor);

				case Quote:
					if (_stack.back() & AcceptValues)
					{
						auto cursor = _cursor;
						const auto quote = *cursor;
						const auto base = ++cursor;
						while (*cursor != quote)
							switch (*cursor)
							{
							case '\0':
							case '\n':
							case '\r':
								return makeError(cursor);
							default:
								++cursor;
							}
						_cursor = cursor + 1;
						return makeToken<-1>(StToken::Type::Value, base, cursor - base);
					}
					else
						return makeError(_cursor);

				case LBracket:
					if (_stack.back() & AcceptValues)
					{
						_stack.emplace_back(AcceptValues);
						return makeToken(StToken::Type::ListBegin, _cursor++, 1);
					}
					else
						return makeError(_cursor);

				case RBracket:
					if (_stack.back() == AcceptValues && _stack.size() > 1)
					{
						_stack.pop_back();
						return makeToken(StToken::Type::ListEnd, _cursor++, 1);
					}
					else
						return makeError(_cursor);

				case LBrace:
					if (_stack.back() & AcceptValues)
					{
						_stack.emplace_back(AcceptKeys);
						return makeToken(StToken::Type::ObjectBegin, _cursor++, 1);
					}
					else
						return makeError(_cursor);

				case RBrace:
					if (_stack.back() & AcceptKeys && _stack.size() > 1)
					{
						_stack.pop_back();
						return makeToken(StToken::Type::ObjectEnd, _cursor++, 1);
					}
					else
						return makeError(_cursor);

				case Comment:
					if (auto next = _cursor + 1; *next == '/')
					{
						_cursor = ::skipWhile(next, [](char c) { return c != '\0' && c != '\n' && c != '\r'; });
						break;
					}
					else
						return makeError(_cursor);
				}
			}
		}

	private:
		constexpr StToken makeError(const char* at) noexcept
		{
			return { _line, static_cast<size_t>(at - _lineBase), StToken::Type::Error, std::string_view{} };
		}

		template <ptrdiff_t columnOffset = 0>
		StToken makeToken(StToken::Type type, const char* begin, ptrdiff_t size) noexcept
		{
			const std::string_view text{ begin, static_cast<size_t>(size) };
			return { _line, static_cast<size_t>(begin - _lineBase + columnOffset), type, text };
		}

	private:
		enum : uint8_t
		{
			AcceptKeys = 1 << 0,
			AcceptValues = 1 << 1,
		};

		const size_t _size;
		Buffer _buffer{ _size + 1 };
		const char* _cursor = reinterpret_cast<const char*>(_buffer.data());
		size_t _line = 1;
		const char* _lineBase = _cursor - 1;
		std::vector<uint8_t> _stack{ AcceptKeys };
	};

	StReader::StReader(const SharedPtr<Blob>& blob)
		: _impl{ std::make_unique<StReaderImpl>(blob) } {}

	StReader::~StReader() noexcept = default;

	StToken StReader::read()
	{
		return _impl->read();
	}
}
