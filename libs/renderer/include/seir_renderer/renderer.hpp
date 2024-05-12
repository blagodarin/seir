// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/shared_ptr.hpp>

#include <functional>
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
		virtual void drawMesh(const Mesh&) = 0;

		//
		virtual void setTransformation(const Mat4&) = 0;
	};

	//
	class Renderer
	{
	public:
		//
		[[nodiscard]] static UniquePtr<Renderer> create(const Window&);

		virtual ~Renderer() noexcept = default;

		//
		[[nodiscard]] virtual SharedPtr<Mesh> createMesh(const MeshFormat&, const void* vertexData, size_t vertexCount, const void* indexData, size_t indexCount) = 0;

		//
		[[nodiscard]] virtual SharedPtr<ShaderSet> createShaders(std::span<const uint32_t> vertexShader, std::span<const uint32_t> fragmentShader) = 0;

		//
		[[nodiscard]] virtual SharedPtr<Texture2D> createTexture2D(const ImageInfo&, const void*) = 0;
		[[nodiscard]] SharedPtr<Texture2D> createTexture2D(const Image&);

		//
		virtual void render(const std::function<Mat4(const Vec2&)>&, const std::function<void(RenderPass&)>&) = 0;
	};
}
