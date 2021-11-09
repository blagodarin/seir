# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

seir_folder(benchmarks)
source_group("src" REGULAR_EXPRESSION ".*\\.(hpp|cpp)$")
add_executable(audio_benchmarks
	src/processing.cpp
	)
target_link_libraries(audio_benchmarks PRIVATE Seir::audio benchmark::benchmark_main)
set_target_properties(audio_benchmarks PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<NOT:$<BOOL:${SEIR_STATIC_RUNTIME}>>:DLL>")
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_compile_options(audio_benchmarks PRIVATE -msse4.1) # Audio benchmarks should have the same compile options as Seir::audio.
endif()
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	target_compile_options(audio_benchmarks PRIVATE -Wno-global-constructors)
endif()
