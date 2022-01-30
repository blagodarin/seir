// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

namespace seir
{
	// Input keys.
	// The values correspond to USB scan codes because why not.
	enum class Key : unsigned char
	{
		None,

		//
		A = 4,
		B,
		C,
		D,
		E,
		F,
		G,
		H,
		I,
		J,
		K,
		L,
		M,
		N,
		O,
		P,
		Q,
		R,
		S,
		T,
		U,
		V,
		W,
		X,
		Y,
		Z,
		_1,
		_2,
		_3,
		_4,
		_5,
		_6,
		_7,
		_8,
		_9,
		_0,
		Enter,
		Escape,
		Backspace,
		Tab,
		Space,
		Minus,
		Equals,
		LBracket,
		RBracket,
		Backslash,
		Hash,
		Semicolon,
		Apostrophe,
		Grave,
		Comma,
		Period,
		Slash,
		CapsLock,

		//
		F1,
		F2,
		F3,
		F4,
		F5,
		F6,
		F7,
		F8,
		F9,
		F10,
		F11,
		F12,
		PrintScreen,
		ScrollLock,
		Pause,

		//
		Insert,
		Home,
		PageUp,
		Delete,
		End,
		PageDown,

		// Arrow keys.
		Right,
		Left,
		Down,
		Up,

		// Numpad keys.
		NumLock,
		Divide,
		Multiply,
		Subtract,
		Add,
		NumEnter,
		Num1,
		Num2,
		Num3,
		Num4,
		Num5,
		Num6,
		Num7,
		Num8,
		Num9,
		Num0,
		Decimal,

		//
		NonUsBackslash,
		App,

		//
		LControl = 224,
		LShift,
		LAlt,
		LGui,
		RControl,
		RShift,
		RAlt,
		RGui,

		// Mouse "keys" don't correspond to any USB scan codes.
		Mouse1 = 240,
		Mouse2,
		Mouse3,
		Mouse4,
		Mouse5,
	};
}
