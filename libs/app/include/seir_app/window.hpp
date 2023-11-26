// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/shared_ptr.hpp>

#include <string>

namespace seir
{
	class App;
	class Image;

	//
	struct Size2D
	{
		int _width = 0;
		int _height = 0;
		constexpr Size2D() noexcept = default;
		constexpr Size2D(int width, int height) noexcept
			: _width{ width }, _height{ height } {}
	};

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
		[[nodiscard]] static UniquePtr<Window> create(const SharedPtr<App>&, const std::string& title);

		virtual ~Window() noexcept = default;

		//
		virtual void close() noexcept = 0;

		//
		[[nodiscard]] virtual WindowDescriptor descriptor() const noexcept = 0;

		//
		virtual void setIcon(const Image&) noexcept = 0;

		//
		virtual void setTitle(const std::string&) noexcept = 0;

		//
		virtual void show() noexcept = 0;

		//
		[[nodiscard]] virtual Size2D size() const noexcept = 0;
	};
}
