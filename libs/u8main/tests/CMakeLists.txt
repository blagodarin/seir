# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(SOURCES
	src/u8main.cpp
	)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_executable(seir_u8main_tests ${SOURCES})
target_link_libraries(seir_u8main_tests PRIVATE Seir::u8main doctest::doctest)
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	target_compile_options(seir_u8main_tests PRIVATE -Wno-exit-time-destructors -Wno-global-constructors)
endif()
seir_target(seir_u8main_tests FOLDER libs/u8main STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
add_test(NAME u8main COMMAND seir_u8main_tests English Русский 日本語)
