# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(SOURCES
	src/archive.cpp
	src/storage.cpp
	)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_executable(seir_package_tests ${SOURCES})
target_compile_definitions(seir_package_tests PRIVATE SEIR_TEST_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/")
target_link_libraries(seir_package_tests PRIVATE Seir::compression Seir::io Seir::package doctest::doctest_with_main)
seir_target(seir_package_tests FOLDER libs/package STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
add_test(NAME package COMMAND seir_package_tests)
