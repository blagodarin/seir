// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/shared_ptr.hpp>

#include <functional>
#include <memory>
#include <span>

namespace seir
{
	class Image;
	class ImageInfo;
	class Mat4;
	class Mesh;
	struct MeshFormat;
	class SizeF;
	class Vec2;
	class Window;

	//
	class ShaderSet : public ReferenceCounter
	{
	public:
		virtual ~ShaderSet() noexcept = default;
	};

	//
	class Texture2D : public ReferenceCounter
	{
	public:
		virtual ~Texture2D() noexcept = default;

		//
		[[nodiscard]] virtual SizeF size() const noexcept = 0;
	};

	//
	class RenderPass
	{
	public:
		virtual ~RenderPass() noexcept = default;

		//
		virtual void bindShaders(const SharedPtr<ShaderSet>&) = 0;

		//
		virtual void bindTexture(const SharedPtr<Texture2D>&) = 0;

		//
		virtual void bindUniformBuffer() = 0; // TODO: Add a parameter.

		//
		virtual void drawMesh(const Mesh&) = 0;

		//
		virtual void setTransformation(const Mat4&) = 0;

		//
		virtual Vec2 size() const noexcept = 0;

		//
		virtual void updateUniformBuffer(const Mat4&) = 0;
	};

	//
	class Renderer
	{
	public:
		//
		Renderer(const Window&);

		~Renderer() noexcept;

		//
		[[nodiscard]] SharedPtr<Mesh> createMesh(const MeshFormat&, const void* vertexData, size_t vertexCount, const void* indexData, size_t indexCount);

		//
		[[nodiscard]] SharedPtr<ShaderSet> createShaders(std::span<const uint32_t> vertexShader, std::span<const uint32_t> fragmentShader);

		//
		[[nodiscard]] SharedPtr<Texture2D> createTexture2D(const ImageInfo&, const void*);
		[[nodiscard]] SharedPtr<Texture2D> createTexture2D(const Image&);

		//
		void render(const std::function<void(RenderPass&)>&);

	private:
		const std::unique_ptr<class RendererImpl> _impl;
	};
}
