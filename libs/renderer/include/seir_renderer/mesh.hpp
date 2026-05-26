// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/shared_ptr.hpp>

namespace seir
{
	//
	class Mesh : public ReferenceCounter
	{
	public:
		virtual ~Mesh() noexcept = default;
	};
}
