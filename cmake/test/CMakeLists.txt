# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.21)
project(SeirTest LANGUAGES CXX)
option(STATIC_RUNTIME "Use static MSVC runtime" OFF)
find_package(Seir REQUIRED)
seir_defaults()
add_executable(seir_test main.cpp)
target_link_libraries(seir_test PRIVATE Seir::base Seir::u8main)
if(NOT APPLE) # macOS builds use libcxx, and libcxx 14 doesn't have all required C++20 features.
	seir_provide_plf_colony(plf_colony_ROOT)
	find_package(plf_colony REQUIRED)
	target_link_libraries(seir_test PRIVATE plf_colony::plf_colony)
endif()
seir_target(seir_test STATIC_RUNTIME ${STATIC_RUNTIME})
