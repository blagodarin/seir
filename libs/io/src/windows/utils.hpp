// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#define NOGDI
#define WIN32_LEAN_AND_MEAN
#include <seir_base/windows_utils.hpp>

#include <array>
#include <string>

namespace seir::windows
{
	class U8String
	{
	public:
		explicit U8String(std::wstring_view path) noexcept
		{
			if (path.empty())
				return;
			const auto length = ::WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, path.data(), static_cast<int>(path.size()), _buffer.data(), static_cast<int>(_buffer.size()), nullptr, nullptr);
			if (!length)
				reportError("WideCharToMultiByte");
			_size = static_cast<size_t>(length);
		}
		constexpr explicit operator bool() const noexcept { return _size > 0; }
		std::string toString() const { return { _buffer.data(), _size }; }

	private:
		size_t _size = 0;
		std::array<char, MAX_PATH> _buffer;
	};

	class WString
	{
	public:
		explicit WString(std::string_view path) noexcept
		{
			if (!path.empty())
			{
				const auto length = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path.data(), static_cast<int>(path.size()), _buffer.data(), static_cast<int>(_buffer.size() - 1));
				if (!length)
					reportError("MultiByteToWideChar");
				_size = static_cast<size_t>(length);
			}
			_buffer[_size] = L'\0';
		}
		constexpr explicit operator bool() const noexcept { return _size > 0; }
		const wchar_t* c_str() const noexcept { return _buffer.data(); }
		size_t size() const noexcept { return _size; }

	private:
		size_t _size = 0;
		std::array<wchar_t, MAX_PATH + 1> _buffer;
	};
}
