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
}

namespace seir
{
	class RendererImpl
	{
	};

	Renderer::Renderer(const Window&)
		: _impl{ std::make_unique<RendererImpl>() }
	{
	}

	Renderer::~Renderer() noexcept = default;

	SharedPtr<Mesh> Renderer::createMesh(const MeshFormat&, const void*, size_t, const void*, size_t)
	{
		return makeShared<Mesh, DummyMesh>();
	}

	SharedPtr<ShaderSet> Renderer::createShaders(std::span<const uint32_t>, std::span<const uint32_t>)
	{
		return makeShared<ShaderSet, DummyShaderSet>();
	}

	SharedPtr<Texture2D> Renderer::createTexture2D(const ImageInfo& info, const void*)
	{
		return makeShared<Texture2D, DummyTexture>(SizeF{ static_cast<float>(info.width()), static_cast<float>(info.height()) });
	}

	void Renderer::render(const std::function<void(RenderPass&)>&)
	{
	}
}
