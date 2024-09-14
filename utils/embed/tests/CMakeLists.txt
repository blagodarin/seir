# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(SOURCES
	src/embed.cpp
	)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_executable(seir_embed_tests ${SOURCES})
target_compile_definitions(seir_embed_tests PRIVATE SEIR_TEST_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/")
target_link_libraries(seir_embed_tests PRIVATE Seir::data doctest::doctest_with_main)
seir_target(seir_embed_tests FOLDER utils/embed STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
seir_target_embed(seir_embed_tests STRING FILES
	data/string.txt
	)
seir_target_embed(seir_embed_tests UINT8 FILES
	data/uint8.txt
	)
add_test(NAME embed COMMAND seir_embed_tests)
