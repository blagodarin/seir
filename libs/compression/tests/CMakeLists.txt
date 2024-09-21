# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(SOURCES
	src/compression.cpp
	)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_executable(seir_compression_tests ${SOURCES})
target_link_libraries(seir_compression_tests PRIVATE Seir::compression doctest::doctest_with_main)
seir_target(seir_compression_tests FOLDER libs/compression STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
add_test(NAME compression COMMAND seir_compression_tests)
