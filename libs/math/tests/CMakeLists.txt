# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(SOURCES
	src/mat.cpp
	src/raster.cpp
	src/vec.cpp
	)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_executable(seir_math_tests ${SOURCES})
target_link_libraries(seir_math_tests PRIVATE Seir::math doctest::doctest_with_main)
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	target_compile_options(seir_math_tests PRIVATE -Wno-double-promotion)
endif()
seir_target(seir_math_tests FOLDER libs/math STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
add_test(NAME math COMMAND seir_math_tests)
