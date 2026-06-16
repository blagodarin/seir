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
	class Vec2;

	//
	struct WindowDescriptor
	{
		void* _pointer = nullptr;
		intptr_t _index = 0;
	};

	//
	struct WindowSize
	{
		uint32_t width = 0;
		uint32_t height = 0;
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
		[[nodiscard]] std::optional<Vec2> cursor() const noexcept;

		//
		[[nodiscard]] WindowDescriptor descriptor() const noexcept;

		//
		void setIcon(const Image&) noexcept;

		//
		void setTitle(const std::string&) noexcept;

		//
		void show() noexcept;

		//
		[[nodiscard]] WindowSize size() const noexcept;

	private:
		const std::unique_ptr<class WindowImpl> _impl;
	};
}
