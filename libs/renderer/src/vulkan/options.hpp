// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

namespace seir
{
	struct RendererOptions
	{
		bool anisotropicFiltering = false;
		bool multisampleAntialiasing = false; // TODO: Support different MSAA levels.
		bool sampleShading = false;           // TODO: Support different sample shading levels.
	};
}
