// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// Called from the main function with UTF-8 arguments.
// Rationale:
// * We want to be able to define a single entry point for all supported platforms.
//   We also want all text to be UTF-8 encoded.
// * Windows GUI applications use WinMain() entry point which requires
//   extra effort to to get and parse the command line.
// * UTF-8 is the default single-byte encoding pretty much everywhere except Windows,
//   so we need to transcode main() arguments on Windows.
int u8main(int argc, char** argv);
