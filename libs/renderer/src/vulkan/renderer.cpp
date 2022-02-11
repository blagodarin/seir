// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_renderer/renderer.hpp>

#include <seir_app/window.hpp>
#include "context.hpp"

namespace
{
	class VulkanRenderer final : public seir::Renderer
	{
	public:
		explicit VulkanRenderer(const seir::SharedPtr<seir::Window>& window) noexcept
			: _window{ window } {}

		bool initialize()
		{
			return _context.initialize(*_window);
		}

		void draw() override
		{
			return _context.draw();
		}

	private:
		const seir::SharedPtr<seir::Window> _window;
		seir::VulkanContext _context;
	};
}

namespace seir
{
	UniquePtr<Renderer> Renderer::create(const SharedPtr<Window>& window)
	{
		if (auto renderer = makeUnique<VulkanRenderer>(window); renderer->initialize())
			return staticCast<Renderer>(std::move(renderer));
		return {};
	}
}
