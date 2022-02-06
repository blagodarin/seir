// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/shared_ptr.hpp>

#include <string>

namespace seir
{
	class App;

	struct Size2D
	{
		int _width = 0;
		int _height = 0;
		constexpr Size2D() noexcept = default;
		constexpr Size2D(int width, int height) noexcept
			: _width{ width }, _height{ height } {}
	};

	//
	class Window : public ReferenceCounter
	{
	public:
		//
		struct Descriptor
		{
			void* _app = nullptr;
			intptr_t _window = 0;
			constexpr Descriptor(void* app, intptr_t window) noexcept
				: _app{ app }, _window{ window } {}
		};

		//
		[[nodiscard]] static UniquePtr<Window> create(const SharedPtr<App>&, const std::string& title);

		//
		virtual void close() noexcept = 0;

		//
		[[nodiscard]] virtual Descriptor descriptor() const noexcept = 0;

		//
		virtual void show() noexcept = 0;

		//
		[[nodiscard]] virtual Size2D size() const noexcept = 0;
	};
}
