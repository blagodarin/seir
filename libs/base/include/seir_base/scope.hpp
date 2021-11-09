// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_base/macros.hpp>

#include <utility>

namespace seir
{
	template <typename Callback>
	class Finally
	{
	public:
		Finally(const Finally&) = delete;
		Finally& operator=(const Finally&) = delete;

		constexpr Finally(Callback&& callback) noexcept
			: _callback{ std::move(callback) } {}

		~Finally() noexcept { _callback(); }

	private:
		const Callback _callback;
	};

	template <typename Callback>
	[[nodiscard]] auto makeFinally(Callback&& callback) noexcept
	{
		return Finally<Callback>{ std::forward<Callback>(callback) };
	}
}

#define SEIR_FINALLY(callback) const auto SEIR_JOIN(seirFinally, __LINE__) = seir::makeFinally(callback)
