// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/buffer.hpp>
#include <seir_model/mesh_format.hpp>

namespace seir
{
	class MeshData
	{
	public:
		MeshData() noexcept;
		MeshData(const MeshData&) = delete;
		MeshData(MeshData&&) noexcept;
		MeshData& operator=(MeshData&) = delete;
		~MeshData() noexcept;

		[[nodiscard]] const MeshFormat& format() const noexcept { return _format; }
		[[nodiscard]] size_t indexCount() const noexcept { return _indexCount; }
		[[nodiscard]] const void* indexData() const noexcept { return _indices.data(); }
		[[nodiscard]] size_t vertexCount() const noexcept { return _vertexCount; }
		[[nodiscard]] const void* vertexData() const noexcept { return _vertices.data(); }

	private:
		MeshFormat _format;
		Buffer _vertices;
		size_t _vertexCount = 0;
		Buffer _indices;
		size_t _indexCount = 0;
	};
}
