# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(SOURCES
	src/allocator.cpp
	src/base64.cpp
	src/base85.cpp
	src/buffer.cpp
	src/endian.cpp
	src/fixed.cpp
	src/int_utils.cpp
	src/intrinsics.cpp
	src/macros.cpp
	src/pointer.cpp
	src/rigid_vector.cpp
	src/scope.cpp
	src/shared_ptr.cpp
	src/static_vector.cpp
	src/string_utils.cpp
	src/unique_ptr.cpp
	src/utf8.cpp
	)
if(WIN32)
	list(APPEND SOURCES
		src/windows_utils.cpp
		)
endif()
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_executable(seir_base_tests ${SOURCES})
target_link_libraries(seir_base_tests PRIVATE Seir::base doctest::doctest_with_main)
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	target_compile_options(seir_base_tests PRIVATE -Wno-self-assign-overloaded -Wno-self-move)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	set_property(SOURCE src/base64.cpp src/base85.cpp APPEND PROPERTY COMPILE_OPTIONS
		/wd4866 # compiler may not enforce left-to-right evaluation order for call to 'operator<<'
		)
endif()
seir_target(seir_base_tests FOLDER libs/base STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
add_test(NAME base COMMAND seir_base_tests)
