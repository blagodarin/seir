// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "common.hpp"

#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

std::filesystem::path thisExecutable;

int main(int argc, char** argv)
{
	thisExecutable = argv[0];
	return doctest::Context(argc, argv).run();
}
