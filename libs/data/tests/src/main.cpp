// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "main.hpp"

#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

std::string thisExecutable;

int main(int argc, char** argv)
{
	thisExecutable = argv[0];
	return doctest::Context(argc, argv).run();
}
