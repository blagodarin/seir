// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/macros.hpp>

#include <type_traits>
#include <utility>

namespace seir
{
	template <typename Callback>
	class Finally
	{
	public:
		static_assert(std::is_nothrow_move_constructible_v<Callback>);
		static_assert(std::is_nothrow_invocable_r_v<void, Callback&&>);

		Finally(const Finally&) = delete;
		Finally& operator=(const Finally&) = delete;
		constexpr explicit Finally(Callback&& callback) noexcept
			: _callback{ std::move(callback) } {}
		~Finally() noexcept { std::move(_callback)(); }

	private:
		Callback _callback;
	};
}

#define SEIR_FINALLY const seir::Finally SEIR_JOIN(seirFinally, __LINE__)
