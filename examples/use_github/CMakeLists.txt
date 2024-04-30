# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.21)
project(UseGithub LANGUAGES CXX)

# Get Seir for the project.
set(SEIR_APP ON)
include(FetchContent)
FetchContent_Declare(Seir GIT_REPOSITORY https://github.com/blagodarin/seir.git GIT_TAG "${REPLACE_WITH_COMMIT_HASH}")
FetchContent_MakeAvailable(Seir)

# Use Seir defaults for some CMake variables.
seir_defaults()

# Do the regular CMake stuff.
add_executable(use_github main.cpp)
target_link_libraries(use_github PRIVATE Seir::app)
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT use_github)
