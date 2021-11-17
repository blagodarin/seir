# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.21)
project(SeirTest LANGUAGES CXX)
option(STATIC_RUNTIME "Use static MSVC runtime" OFF)
find_package(Seir REQUIRED)
seir_defaults()
seir_provide_plf_colony(plf_colony_ROOT)
find_package(plf_colony REQUIRED)
add_executable(seir_test main.cpp)
target_link_libraries(seir_test PRIVATE Seir::base Seir::main plf_colony::plf_colony)
seir_target(seir_test STATIC_RUNTIME ${STATIC_RUNTIME})
