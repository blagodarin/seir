// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_app/window.hpp>

#include <seir_app/app.hpp>
#include <seir_graphics/rect.hpp>

namespace
{
	class StubWindow : public seir::Window
	{
	public:
		explicit StubWindow(const seir::SharedPtr<seir::App>& app) noexcept
			: _app{ app } {}

		void close() noexcept override { _app->quit(); }
		std::optional<seir::Point> cursor() const noexcept override { return {}; }
		seir::WindowDescriptor descriptor() const noexcept override { return { nullptr, 0 }; }
		void setIcon(const seir::Image&) noexcept override {}
		void setTitle(const std::string&) noexcept override {}
		void show() noexcept override {}
		seir::Size size() const noexcept override { return {}; }

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
