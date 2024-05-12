// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_renderer/renderer.hpp>

#include <seir_graphics/sizef.hpp>
#include <seir_image/image.hpp>
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
	public:
		DummyTexture(const seir::SizeF& size) noexcept
			: _size{ size } {}
		seir::SizeF size() const noexcept override { return _size; }

	private:
		const seir::SizeF _size;
	};

	class DummyRenderer final : public seir::Renderer
	{
	public:
		seir::UniquePtr<seir::Mesh> createMesh(const seir::MeshFormat&, const void*, size_t, const void*, size_t) override
		{
			return seir::makeUnique<seir::Mesh, DummyMesh>();
		}

		seir::SharedPtr<seir::ShaderSet> createShaders(std::span<const uint32_t>, std::span<const uint32_t>) override
		{
			return seir::makeShared<seir::ShaderSet, DummyShaderSet>();
		}

		seir::SharedPtr<seir::Texture2D> createTexture2D(const seir::ImageInfo& info, const void*) override
		{
			return seir::makeShared<seir::Texture2D, DummyTexture>(seir::SizeF{ static_cast<float>(info.width()), static_cast<float>(info.height()) });
		}

		void render(const std::function<seir::Mat4(const seir::Vec2&)>&, const std::function<void(seir::RenderPass&)>&) override {}
	};
}

namespace seir
{
	UniquePtr<Renderer> Renderer::create(const Window&)
	{
		return makeUnique<Renderer, DummyRenderer>();
	}
}
