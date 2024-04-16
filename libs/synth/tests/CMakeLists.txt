# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(SOURCES
	src/acoustics.cpp
	src/common.cpp
	src/format.cpp
	src/loop.cpp
	src/modulator.cpp
	src/oscillator.cpp
	src/period.cpp
	src/tables.cpp
	src/voice.cpp
	src/wave.cpp
	)
if(NOT APPLE) # TODO: Fix Shaper tests for ARM64.
	list(APPEND SOURCES
		src/shaper.cpp
		)
endif()
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_executable(seir_synth_tests ${SOURCES})
target_link_libraries(seir_synth_tests PRIVATE Seir::synth Seir::base doctest::doctest_with_main)
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	target_compile_options(seir_synth_tests PRIVATE -Wno-double-promotion -Wno-float-equal)
endif()
seir_target(seir_synth_tests FOLDER libs/synth STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
add_test(NAME synth COMMAND seir_synth_tests)
