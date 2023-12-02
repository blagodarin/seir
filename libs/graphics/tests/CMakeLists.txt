# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(SOURCES
	src/color.cpp
	src/point.cpp
	src/rect.cpp
	src/size.cpp
	)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_executable(seir_graphics_tests ${SOURCES})
target_link_libraries(seir_graphics_tests PRIVATE Seir::graphics doctest::doctest_with_main)
seir_target(seir_graphics_tests FOLDER libs/graphics STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
add_test(NAME graphics COMMAND seir_graphics_tests)
