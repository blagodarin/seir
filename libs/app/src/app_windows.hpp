// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_app/app.hpp>
#include "app.hpp"

#define WIN32_LEAN_AND_MEAN
#include <seir_base/windows_utils.hpp>

namespace seir
{
	class WindowsApp : public App
		, private AppHelper
	{
	public:
		[[nodiscard]] constexpr HINSTANCE instance() const noexcept { return _instance; }

	protected:
		bool processEvents() override;
		void quit() override;

	private:
		const HINSTANCE _instance = ::GetModuleHandleW(nullptr);
	};
}
