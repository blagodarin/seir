# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

seir_folder(base)
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
add_executable(base_tests ${SOURCES})
target_link_libraries(base_tests PRIVATE Seir::base doctest::doctest_with_main)
set_target_properties(base_tests PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<NOT:$<BOOL:${SEIR_STATIC_RUNTIME}>>:DLL>")
add_test(NAME base COMMAND base_tests)
