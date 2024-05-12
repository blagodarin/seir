// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/shared_ptr.hpp>

#include <optional>
#include <string>

namespace seir
{
	class App;
	class Image;
	class Point;
	class Size;

	//
	struct WindowDescriptor
	{
		void* _app = nullptr;
		intptr_t _window = 0;
		constexpr WindowDescriptor(void* app, intptr_t window) noexcept
			: _app{ app }, _window{ window } {}
	};

	//
	class Window : public ReferenceCounter
	{
	public:
		//
		[[nodiscard]] static UniquePtr<Window> create(App&, const std::string& title);

		virtual ~Window() noexcept = default;

		//
		virtual void close() noexcept = 0;

		//
		virtual std::optional<Point> cursor() const noexcept = 0;

		//
		[[nodiscard]] virtual WindowDescriptor descriptor() const noexcept = 0;

		//
		virtual void setIcon(const Image&) noexcept = 0;

		//
		virtual void setTitle(const std::string&) noexcept = 0;

		//
		virtual void show() noexcept = 0;

		//
		[[nodiscard]] virtual Size size() const noexcept = 0;
	};
}
