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
		f32x2, // Two 32-bit floats.
		f32x3, // Three 32-bit floats.
		un8x4, // Four 8-bit unsigned integers which get converted into 32-bit floats in [0, 1] range.
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
