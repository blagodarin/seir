# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.21)
project(Example LANGUAGES CXX)

# Disable unused components to minimize CI build times.
set(SEIR_AUDIO OFF)
set(SEIR_GUI OFF)
set(SEIR_RENDERER OFF)
set(SEIR_SERIALIZATION OFF)
set(SEIR_SYNTH OFF)
set(SEIR_U8MAIN OFF)
set(SEIR_UTILS OFF)

# Get Seir for the project.
include(FetchContent)
FetchContent_Declare(Seir GIT_REPOSITORY https://github.com/blagodarin/seir.git GIT_TAG "${REPLACE_WITH_COMMIT_HASH}")
FetchContent_MakeAvailable(Seir)

# Use Seir defaults for some CMake variables.
seir_defaults()

# Do the regular CMake stuff.
add_executable(cmake_fetchcontent main.cpp)
target_link_libraries(cmake_fetchcontent PRIVATE Seir::app)
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT cmake_fetchcontent)
