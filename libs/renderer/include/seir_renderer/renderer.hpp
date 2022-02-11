// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

namespace seir
{
	template <class>
	class SharedPtr;
	template <class>
	class UniquePtr;
	class Window;

	//
	class Renderer
	{
	public:
		//
		[[nodiscard]] static UniquePtr<Renderer> create(const SharedPtr<Window>&);

		virtual ~Renderer() noexcept = default;

		//
		virtual void draw() = 0;
	};
}
