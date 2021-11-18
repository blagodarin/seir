// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// This header is intentionally kept without any dependencies except <windows.h>.

#define NOBITMAP
#define NOGDI
#define NOIME
#define NOKERNEL
#define NOMCX
#define NOMINMAX
#define NOSERVICE
#pragma warning(push)
#pragma warning(disable : 4668) // '___' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
#pragma warning(disable : 5039) // pointer or reference to potentially throwing function passed to 'extern "C"' function under -EHc. Undefined behavior may occur if this function throws an exception.
#include <windows.h>
#pragma warning(pop)

namespace seir
{
	namespace windows
	{
		// RAII wrapper for HANDLE.
		class Handle
		{
		public:
			constexpr Handle() noexcept = default;
			explicit constexpr Handle(HANDLE handle) noexcept
				: _handle{ handle } {}
			constexpr Handle(Handle&& other) noexcept
				: _handle{ other._handle } { other._handle = nullptr; }
			inline ~Handle() noexcept;
			constexpr Handle& operator=(Handle&&) noexcept;
			[[nodiscard]] constexpr operator HANDLE() const noexcept { return _handle; }

		private:
			HANDLE _handle = nullptr;
		};

		// RAII wrapper for "local memory objects".
		template <typename T>
		class LocalPtr
		{
		public:
			constexpr LocalPtr() noexcept = default;
			explicit constexpr LocalPtr(T* data) noexcept
				: _data{ data } {}
			constexpr LocalPtr(LocalPtr&& other) noexcept
				: _data{ other._data } { other._data = nullptr; }
			~LocalPtr() noexcept { ::LocalFree(_data); }
			[[nodiscard]] constexpr operator const T*() const noexcept { return _data; }

		private:
			T* _data = nullptr;
		};

		inline LocalPtr<char> errorText(DWORD error) noexcept
		{
			char* buffer = nullptr;
			auto length = ::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				nullptr, error, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), reinterpret_cast<char*>(&buffer), 0, nullptr);
			if (length > 0)
			{
				if (buffer[length - 1] == '\n')
				{
					--length;
					if (length > 0 && buffer[length - 1] == '\r')
						--length;
					buffer[length] = '\0';
				}
			}
			return LocalPtr<char>{ buffer };
		}

		inline void reportLastError(const char* function) noexcept
		{
			if (const auto message = errorText(::GetLastError()))
			{
				DWORD_PTR arguments[]{ (DWORD_PTR)function, (DWORD_PTR) static_cast<const char*>(message) };
				char* buffer = nullptr;
				if (::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
						"[::%1] %2%n", 0, 0, reinterpret_cast<char*>(&buffer), 0, reinterpret_cast<va_list*>(arguments)))
				{
					::OutputDebugStringA(buffer);
					::LocalFree(buffer);
				}
			}
		}
	}
}

seir::windows::Handle::~Handle() noexcept
{
	if (_handle && _handle != INVALID_HANDLE_VALUE && !::CloseHandle(_handle))
		reportLastError("CloseHandle");
}

constexpr seir::windows::Handle& seir::windows::Handle::operator=(seir::windows::Handle&& other) noexcept
{
	const auto handle = _handle;
	_handle = other._handle;
	other._handle = handle;
	return *this;
}
