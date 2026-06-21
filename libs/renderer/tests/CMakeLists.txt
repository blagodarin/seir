# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(SOURCES
	src/color.cpp
	)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_executable(seir_renderer_tests ${SOURCES})
target_link_libraries(seir_renderer_tests PRIVATE Seir::renderer doctest::doctest_with_main)
seir_target(seir_renderer_tests FOLDER libs/renderer STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
add_test(NAME renderer COMMAND seir_renderer_tests)
