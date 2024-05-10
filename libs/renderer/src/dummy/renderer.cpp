// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_renderer/renderer.hpp>

#include <seir_app/window.hpp>
#include <seir_renderer/mesh.hpp>

namespace
{
	class DummyMesh final : public seir::Mesh
	{
	};

	class DummyShaderSet final : public seir::ShaderSet
	{
	};

	class DummyTexture final : public seir::Texture2D
	{
	};

	class DummyRenderer final : public seir::Renderer
	{
	public:
		explicit DummyRenderer(const seir::SharedPtr<seir::Window>& window) noexcept
			: _window{ window } {}

		seir::UniquePtr<seir::Mesh> createMesh(const seir::MeshFormat&, const void*, size_t, const void*, size_t) override { return seir::makeUnique<seir::Mesh, DummyMesh>(); }
		seir::SharedPtr<seir::ShaderSet> createShaders(std::span<const uint32_t>, std::span<const uint32_t>) override { return seir::makeShared<seir::ShaderSet, DummyShaderSet>(); }
		seir::SharedPtr<seir::Texture2D> createTexture2D(const seir::ImageInfo&, const void*) override { return seir::makeShared<seir::Texture2D, DummyTexture>(); }
		void render(const std::function<seir::Mat4(const seir::Vec2&)>&, const std::function<void(seir::RenderPass&)>&) override {}

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
