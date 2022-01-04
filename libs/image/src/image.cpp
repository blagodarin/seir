// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_image/image.hpp>

#include <seir_data/blob.hpp>

namespace seir
{
	std::optional<Image> Image::load(const SharedPtr<Blob>&)
	{
		return {};
	}

	Image::Image() noexcept = default;
	Image::Image(Image&&) noexcept = default;
	Image& Image::operator=(Image&&) noexcept = default;
	Image::~Image() noexcept = default;
}
