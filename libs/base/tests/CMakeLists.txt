# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(SOURCES
	src/allocator.cpp
	src/buffer.cpp
	src/endian.cpp
	src/fixed.cpp
	src/intrinsics.cpp
	src/macros.cpp
	src/pointer.cpp
	src/rigid_vector.cpp
	src/scope.cpp
	src/static_vector.cpp
	src/string_utils.cpp
	src/utf8.cpp
	)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_executable(seir_base_tests ${SOURCES})
target_link_libraries(seir_base_tests PRIVATE Seir::base doctest::doctest_with_main)
seir_target(seir_base_tests FOLDER base STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
add_test(NAME base COMMAND seir_base_tests)
