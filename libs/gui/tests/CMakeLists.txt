# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(SOURCES
	src/font.cpp
	)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_executable(seir_gui_tests ${SOURCES})
target_compile_definitions(seir_gui_tests PRIVATE SEIR_TEST_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/")
target_link_libraries(seir_gui_tests PRIVATE Seir::data Seir::gui doctest::doctest_with_main)
seir_target(seir_gui_tests FOLDER libs/gui STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
add_test(NAME gui COMMAND seir_gui_tests)
