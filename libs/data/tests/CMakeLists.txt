# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(SOURCES
	src/blob.cpp
	src/common.hpp
	src/file.cpp
	src/main.cpp
	src/reader.cpp
	src/storage.cpp
	)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_executable(seir_data_tests ${SOURCES})
target_link_libraries(seir_data_tests PRIVATE Seir::data doctest::doctest)
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	target_compile_options(seir_data_tests PRIVATE -Wno-exit-time-destructors -Wno-global-constructors)
endif()
seir_target(seir_data_tests FOLDER libs/data STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
add_test(NAME data COMMAND seir_data_tests)
