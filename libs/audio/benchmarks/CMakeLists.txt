# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(SOURCES
	src/processing.cpp
	)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_executable(seir_audio_benchmarks ${SOURCES})
target_link_libraries(seir_audio_benchmarks PRIVATE Seir::audio benchmark::benchmark_main)
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	target_compile_options(seir_audio_benchmarks PRIVATE -Wno-global-constructors)
endif()
seir_target(seir_audio_benchmarks FOLDER libs/audio STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
