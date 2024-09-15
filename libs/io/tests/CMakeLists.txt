# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(SOURCES
	src/blob.cpp
	src/buffer_blob.cpp
	src/buffer_writer.cpp
	src/reader.cpp
	src/save_file.cpp
	src/temporary.cpp
	src/writer.cpp
	)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_executable(seir_io_tests ${SOURCES})
target_compile_definitions(seir_io_tests PRIVATE SEIR_TEST_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/")
target_link_libraries(seir_io_tests PRIVATE Seir::io doctest::doctest_with_main)
seir_target(seir_io_tests FOLDER libs/io STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
add_test(NAME io COMMAND seir_io_tests)
