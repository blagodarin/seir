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
		seir::WindowDescriptor descriptor() const noexcept override { return { nullptr, 0 }; }
		void setTitle(const std::string&) noexcept override {}
		void show() noexcept override {}
		seir::Size2D size() const noexcept override { return {}; }

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
