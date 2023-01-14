// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/shared_ptr.hpp>
#include <seir_base/static_vector.hpp>

namespace seir
{
	//
	enum class VertexAttribute
	{
		f32x2, //
		f32x3, //
	};

	//
	enum class MeshIndexType
	{
		u16, //
		u32, //
	};

	//
	enum class MeshTopology
	{
		TriangleList,  //
		TriangleStrip, //
	};

	//
	struct MeshFormat
	{
		StaticVector<VertexAttribute, 6> vertexAttributes;
		MeshTopology topology;
		MeshIndexType indexType;
	};

	//
	class Mesh : public ReferenceCounter
	{
	public:
		virtual ~Mesh() noexcept = default;
	};
}
