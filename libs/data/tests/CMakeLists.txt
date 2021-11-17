# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(SOURCES
	src/blob.cpp
	src/main.cpp
	src/main.hpp
	)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_executable(seir_data_tests ${SOURCES})
target_link_libraries(seir_data_tests PRIVATE Seir::data doctest::doctest)
seir_target(seir_data_tests FOLDER data STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
add_test(NAME data COMMAND seir_data_tests)
