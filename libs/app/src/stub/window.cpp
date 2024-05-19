// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_app/window.hpp>

#include <seir_app/app.hpp>
#include <seir_graphics/point.hpp>
#include <seir_graphics/size.hpp>

namespace seir
{
	class WindowImpl
	{
	public:
		App& _app;
		explicit WindowImpl(App& app) noexcept
			: _app{ app } {}
	};

	Window::Window(App& app, const std::string&)
		: _impl{ std::make_unique<WindowImpl>(app) }
	{
	}

	Window::~Window() noexcept = default;

	void Window::close() noexcept
	{
		_impl->_app.quit();
	}

	std::optional<Point> Window::cursor() const noexcept
	{
		return {};
	}

	WindowDescriptor Window::descriptor() const noexcept
	{
		return { nullptr, 0 };
	}

	void Window::setIcon(const Image&) noexcept
	{
	}

	void Window::setTitle(const std::string&) noexcept
	{
	}

	void Window::show() noexcept
	{
	}

	Size Window::size() const noexcept
	{
		return {};
	}
}
