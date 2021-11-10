// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_base/string_utils.hpp>

#include <iostream>

int main()
{
	std::string helloWorld = "Hello world!";
	seir::normalizeWhitespace(helloWorld, seir::TrailingSpace::Remove);
	std::cerr << helloWorld << '\n';
}
