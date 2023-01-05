// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_renderer/renderer.hpp>

#include <seir_app/window.hpp>

namespace
{
	class DummyRenderer final : public seir::Renderer
	{
	public:
		explicit DummyRenderer(const seir::SharedPtr<seir::Window>& window) noexcept
			: _window{ window } {}

		seir::UniquePtr<seir::Mesh> createMesh(const void*, size_t, size_t, const void*, seir::Mesh::IndexType, size_t) override { return {}; }
		seir::UniquePtr<seir::Texture2D> createTexture2D(const seir::ImageInfo&, const void*) override { return {}; }
		void render(const std::function<void(const seir::Vec2&, seir::RenderPass&)>&) override {}

	private:
		const seir::SharedPtr<seir::Window> _window;
	};
}

namespace seir
{
	UniquePtr<Renderer> Renderer::create(const SharedPtr<Window>& window)
	{
		return makeUnique<Renderer, DummyRenderer>(window);
	}
}
