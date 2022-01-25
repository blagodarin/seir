// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

namespace seir
{
	class App;
	template <class>
	class SharedPtr;
	template <class>
	class UniquePtr;

	//
	class Window
	{
	public:
		//
		[[nodiscard]] static UniquePtr<Window> create(const SharedPtr<App>&);

		virtual ~Window() noexcept = default;

		//
		virtual void close() noexcept = 0;

		//
		virtual void show() noexcept = 0;
	};
}
