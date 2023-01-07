// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/shared_ptr.hpp>

#include <functional>

namespace seir
{
	class Image;
	class ImageInfo;
	class Mat4;
	class Vec2;
	class Window;

	//
	class Mesh : public ReferenceCounter
	{
	public:
		//
		enum class IndexType
		{
			U16, //
			U32, //
		};

		virtual ~Mesh() noexcept = default;
	};

	//
	class Texture2D : public ReferenceCounter
	{
	public:
		virtual ~Texture2D() noexcept = default;
	};

	//
	class RenderPass
	{
	public:
		virtual ~RenderPass() noexcept = default;

		//
		virtual void bindTexture(const SharedPtr<Texture2D>&) = 0;

		//
		virtual void drawMesh(const Mesh&) = 0;

		//
		virtual void setProjection(const Mat4& projection, const Mat4& view) = 0;

		//
		virtual void setTransformation(const Mat4&) = 0;
	};

	//
	class Renderer
	{
	public:
		//
		[[nodiscard]] static UniquePtr<Renderer> create(const SharedPtr<Window>&);

		virtual ~Renderer() noexcept = default;

		//
		[[nodiscard]] virtual UniquePtr<Mesh> createMesh(const void* vertexData, size_t vertexSize, size_t vertexCount, const void* indexData, Mesh::IndexType, size_t indexCount) = 0;

		//
		[[nodiscard]] virtual SharedPtr<Texture2D> createTexture2D(const ImageInfo&, const void*) = 0;
		[[nodiscard]] SharedPtr<Texture2D> createTexture2D(const Image&);

		//
		virtual void render(const std::function<void(const Vec2&, RenderPass&)>&) = 0;
	};
}
