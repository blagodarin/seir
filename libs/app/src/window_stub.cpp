// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_app/window.hpp>

#include <seir_app/app.hpp>

namespace
{
	class StubWindow : public seir::Window
	{
	public:
		StubWindow(const seir::SharedPtr<seir::App>& app) noexcept
			: _app{ app } {}

		void close() noexcept override { _app->quit(); }
		void show() noexcept override {}

	private:
		const seir::SharedPtr<seir::App> _app;
	};
}

namespace seir
{
	UniquePtr<Window> Window::create(const SharedPtr<App>& app)
	{
		return makeUnique<Window, StubWindow>(app);
	}
}
