// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/shared_ptr.hpp>

#include <string>

namespace seir
{
	class App;

	//
	class Window : public ReferenceCounter
	{
	public:
		//
		[[nodiscard]] static UniquePtr<Window> create(const SharedPtr<App>&, const std::string& title);

		//
		virtual void close() noexcept = 0;

		//
		virtual void show() noexcept = 0;
	};
}
