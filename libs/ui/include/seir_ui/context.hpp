// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <memory>

namespace seir
{
	class EventCallbacks;
	class Font;
	template <class>
	class SharedPtr;
	class Window;

	class UiContext
	{
	public:
		explicit UiContext(Window&);
		UiContext(Window&, const SharedPtr<Font>&);
		~UiContext() noexcept;

		EventCallbacks& eventCallbacks() noexcept;
		void setDefaultFont(const SharedPtr<Font>&) noexcept;

	private:
		const std::unique_ptr<class UiContextImpl> _impl;
		friend class UiFrame;
	};
}
