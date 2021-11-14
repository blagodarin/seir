// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_base/string_utils.hpp>
#include <seir_main/u8main.hpp>

#include <iostream>

int u8main(int, char**)
{
	std::string helloWorld = "Hello world!";
	seir::normalizeWhitespace(helloWorld, seir::TrailingSpace::Remove);
	std::cerr << helloWorld << '\n';
	return 0;
}
