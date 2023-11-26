// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/pointer.hpp>

#define WIN32_LEAN_AND_MEAN
#include <seir_base/windows_utils.hpp>

namespace seir
{
	struct HcursorDeleter
	{
		static void free(HCURSOR) noexcept;
	};

	struct HiconDeleter
	{
		static void free(HICON) noexcept;
	};

	struct HwndDeleter
	{
		static void free(HWND) noexcept;
	};

	using Hcursor = Pointer<std::remove_pointer_t<HCURSOR>, HcursorDeleter>;
	using Hicon = Pointer<std::remove_pointer_t<HICON>, HiconDeleter>;
	using Hwnd = Pointer<std::remove_pointer_t<HWND>, HwndDeleter>;
}
