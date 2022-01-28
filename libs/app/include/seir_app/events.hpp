// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

namespace seir
{
	class Window;

	//
	struct KeyEvent
	{
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
	};
}
