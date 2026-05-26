// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_model/mesh_data.hpp>

#include <doctest/doctest.h>

TEST_CASE("MeshData")
{
	seir::MeshData mesh;
	CHECK(mesh.indexCount() == 0);
	CHECK_FALSE(mesh.indexData());
	CHECK(mesh.vertexCount() == 0);
	CHECK_FALSE(mesh.vertexData());
}
