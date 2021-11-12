# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

seir_folder(audio)
set(SOURCES
	src/backend.cpp
	src/common.hpp
	src/player.cpp
	src/processing.cpp
	)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_executable(seir_audio_tests ${SOURCES})
target_link_libraries(seir_audio_tests PRIVATE Seir::audio doctest::doctest_with_main)
set_target_properties(seir_audio_tests PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<NOT:$<BOOL:${SEIR_STATIC_RUNTIME}>>:DLL>")
add_test(NAME audio COMMAND seir_audio_tests)
