# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.21)
project(SeirExample LANGUAGES CXX)
option(STATIC_RUNTIME "Use static runtime" OFF)
find_package(Seir REQUIRED)
seir_set_defaults()
add_executable(seir_example main.cpp)
target_link_libraries(seir_example PRIVATE Seir::base)
set_target_properties(seir_example PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<NOT:$<BOOL:${STATIC_RUNTIME}>>:DLL>")
