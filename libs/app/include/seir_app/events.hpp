// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_app/key.hpp>

#include <string_view>

namespace seir
{
	class Window;

	//
	struct KeyEvent
	{
		Key _key = Key::None;
		bool _pressed = false;
		bool _repeated = false;
	};

	//
	class EventCallbacks
	{
	public:
		virtual ~EventCallbacks() noexcept = default;

		//
		virtual void onKeyEvent(Window&, const KeyEvent&) {}

		//
		virtual void onTextEvent(Window&, std::string_view) {}
	};
}
