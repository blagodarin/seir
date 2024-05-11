// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// <vulkan.h> shamelessly includes <windows.h>,
// so we should prevent it from pulling too much garbage.
#ifdef _WIN32
#	define NOGDI
#	define NOUSER
#	define WIN32_LEAN_AND_MEAN
#	include <seir_base/windows_utils.hpp>
#endif

#include <vulkan/vulkan.h>
