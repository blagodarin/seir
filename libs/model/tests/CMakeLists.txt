# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(SOURCES
	src/mesh_data.cpp
	)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_executable(seir_model_tests ${SOURCES})
target_compile_definitions(seir_model_tests PRIVATE SEIR_TEST_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/")
target_link_libraries(seir_model_tests PRIVATE Seir::io Seir::model doctest::doctest_with_main)
seir_target(seir_model_tests FOLDER libs/model STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
add_test(NAME model COMMAND seir_model_tests)
