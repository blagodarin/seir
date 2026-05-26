// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_model/mesh_data.hpp>

namespace seir
{
	MeshData::MeshData() noexcept = default;
	MeshData::MeshData(MeshData&&) noexcept = default;
	MeshData& MeshData::operator=(MeshData&&) noexcept = default;
	MeshData::~MeshData() noexcept = default;
}
