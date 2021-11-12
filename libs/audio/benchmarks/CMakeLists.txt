# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

seir_folder(audio)
set(SOURCES
	src/processing.cpp
	)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_executable(seir_audio_benchmarks ${SOURCES})
target_link_libraries(seir_audio_benchmarks PRIVATE Seir::audio benchmark::benchmark_main)
set_target_properties(seir_audio_benchmarks PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<NOT:$<BOOL:${SEIR_STATIC_RUNTIME}>>:DLL>")
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_compile_options(seir_audio_benchmarks PRIVATE -msse4.1) # Audio benchmarks should have the same compile options as Seir::audio.
endif()
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	target_compile_options(seir_audio_benchmarks PRIVATE -Wno-global-constructors)
endif()
