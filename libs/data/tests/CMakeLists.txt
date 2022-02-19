# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(SOURCES
	src/archive.cpp
	src/blob.cpp
	src/buffer_blob.cpp
	src/buffer_writer.cpp
	src/compression.cpp
	src/reader.cpp
	src/save_file.cpp
	src/storage.cpp
	src/temporary.cpp
	src/writer.cpp
	)
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	set_property(SOURCE src/archive.cpp src/reader.cpp src/save_file.cpp APPEND PROPERTY COMPILE_OPTIONS
		/wd4866 # compiler may not enforce left-to-right evaluation order for call to 'operator<<'
		)
endif()
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_executable(seir_data_tests ${SOURCES})
target_compile_definitions(seir_data_tests PRIVATE SEIR_TEST_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/")
target_link_libraries(seir_data_tests PRIVATE Seir::data doctest::doctest_with_main)
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	target_compile_options(seir_data_tests PRIVATE -Wno-exit-time-destructors -Wno-global-constructors)
endif()
seir_target(seir_data_tests FOLDER libs/data STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
add_test(NAME data COMMAND seir_data_tests)
