// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <seir_data/writer.hpp>

namespace seir
{
	//
	class TemporaryFile
	{
	public:
		virtual ~TemporaryFile() = default;

		//
		[[nodiscard]] const std::string& path() const noexcept { return _path; }

	protected:
		const std::string _path;
		TemporaryFile(std::string&& path) noexcept
			: _path{ std::move(path) } {}
	};

	//
	class TemporaryWriter : public Writer
	{
	public:
		//
		[[nodiscard]] static UniquePtr<TemporaryWriter> create();

		//
		[[nodiscard]] static UniquePtr<TemporaryFile> commit(UniquePtr<TemporaryWriter>&&);
	};
}
