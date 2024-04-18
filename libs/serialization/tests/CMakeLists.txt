# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(SOURCES
	src/st_reader.cpp
	src/st_stream.cpp
	src/st_writer.cpp
	)
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	set_property(SOURCE src/st_reader.cpp APPEND PROPERTY COMPILE_OPTIONS
		/wd4868 # compiler may not enforce left-to-right evaluation order in braced initializer list
		)
endif()
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_executable(seir_serialization_tests ${SOURCES})
target_link_libraries(seir_serialization_tests PRIVATE Seir::data Seir::serialization doctest::doctest_with_main)
seir_target(seir_serialization_tests FOLDER libs/serialization STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
add_test(NAME serialization COMMAND seir_serialization_tests)
