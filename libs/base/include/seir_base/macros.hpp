// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#define SEIR_JOIN_(left, right) left##right
#define SEIR_JOIN(left, right) SEIR_JOIN_(left, right)
