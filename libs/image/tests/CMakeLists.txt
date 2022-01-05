# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(SOURCES
	src/format.cpp
	src/image.cpp
	src/image.hpp
	)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_executable(seir_image_tests ${SOURCES})
target_compile_definitions(seir_image_tests PRIVATE SEIR_TEST_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/")
target_link_libraries(seir_image_tests PRIVATE Seir::data Seir::image doctest::doctest_with_main)
seir_target(seir_image_tests FOLDER libs/image STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
add_test(NAME image COMMAND seir_image_tests)
