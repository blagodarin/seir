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
		explicit StubWindow(const seir::SharedPtr<seir::App>& app) noexcept
			: _app{ app } {}

		void close() noexcept override { _app->quit(); }
		seir::Window::Descriptor descriptor() const noexcept override { return { nullptr, 0 }; }
		void show() noexcept override {}

	private:
		const seir::SharedPtr<seir::App> _app;
	};
}

namespace seir
{
	UniquePtr<Window> Window::create(const SharedPtr<App>& app, const std::string&)
	{
		return makeUnique<Window, StubWindow>(app);
	}
}
