# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.31)
project(Example LANGUAGES CXX)
option(STATIC_RUNTIME "Use static MSVC runtime" OFF)
find_package(Seir REQUIRED)
seir_defaults()
seir_provide_plf_colony(plf_colony_ROOT)
find_package(plf_colony REQUIRED)
add_executable(cmake_package main.cpp)
target_link_libraries(cmake_package PRIVATE Seir::base Seir::u8main plf_colony::plf_colony)
seir_target(cmake_package STATIC_RUNTIME ${STATIC_RUNTIME})
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT cmake_package)
