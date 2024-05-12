// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <optional>
#include <memory>
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
	class Window
	{
	public:
		//
		Window(App&, const std::string& title);

		~Window() noexcept;

		//
		void close() noexcept;

		//
		[[nodiscard]] std::optional<Point> cursor() const noexcept;

		//
		[[nodiscard]] WindowDescriptor descriptor() const noexcept;

		//
		void setIcon(const Image&) noexcept;

		//
		void setTitle(const std::string&) noexcept;

		//
		void show() noexcept;

		//
		[[nodiscard]] Size size() const noexcept;

	private:
		const std::unique_ptr<class WindowImpl> _impl;
	};
}
