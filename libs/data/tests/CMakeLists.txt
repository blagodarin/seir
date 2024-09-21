# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(SOURCES
	src/archive.cpp
	src/storage.cpp
	)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_executable(seir_data_tests ${SOURCES})
target_compile_definitions(seir_data_tests PRIVATE SEIR_TEST_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/")
target_link_libraries(seir_data_tests PRIVATE Seir::compression Seir::data Seir::io doctest::doctest_with_main)
seir_target(seir_data_tests FOLDER libs/data STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
add_test(NAME data COMMAND seir_data_tests)
