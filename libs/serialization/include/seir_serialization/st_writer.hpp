// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace seir
{
	//
	class StWriter
	{
	public:
		//
		struct UnexpectedToken : public std::logic_error
		{
			UnexpectedToken()
				: std::logic_error{ "Unexpected token" } {}
		};

		//
		enum class Formatting
		{
			Compact, //
			Pretty,  //
		};

		//
		StWriter(std::string& buffer, Formatting);

		~StWriter() noexcept;

		//
		void addKey(std::string_view);

		//
		void addValue(std::string_view);

		//
		void beginList();

		//
		void beginObject();

		//
		void endList();

		//
		void endObject();

		//
		void finish();

	private:
		void beginPrettyValue(uint8_t);

	private:
		std::string& _buffer;
		std::vector<uint8_t> _stack;
		const bool _pretty;
	};
}
