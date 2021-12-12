# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(SOURCES
	src/backend.cpp
	src/common.hpp
	src/decoder.cpp
	src/player.cpp
	src/processing.cpp
	)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_executable(seir_audio_tests ${SOURCES})
target_compile_definitions(seir_audio_tests PRIVATE SEIR_TEST_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/")
target_link_libraries(seir_audio_tests PRIVATE Seir::audio Seir::base doctest::doctest_with_main)
seir_target(seir_audio_tests FOLDER libs/audio STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
add_test(NAME audio COMMAND seir_audio_tests)
