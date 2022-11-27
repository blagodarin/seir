# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(HEADERS
	include/seir_base/allocator.hpp
	include/seir_base/base64.hpp
	include/seir_base/base85.hpp
	include/seir_base/buffer.hpp
	include/seir_base/endian.hpp
	include/seir_base/fixed.hpp
	include/seir_base/int_utils.hpp
	include/seir_base/intrinsics.hpp
	include/seir_base/macros.hpp
	include/seir_base/pointer.hpp
	include/seir_base/rigid_vector.hpp
	include/seir_base/scope.hpp
	include/seir_base/shared_ptr.hpp
	include/seir_base/static_vector.hpp
	include/seir_base/string_utils.hpp
	include/seir_base/unique_ptr.hpp
	include/seir_base/utf8.hpp
	)
set(SOURCES
	src/allocator.cpp
	src/buffer.cpp
	)
if(WIN32)
	list(APPEND HEADERS
		include/seir_base/windows_utils.hpp
		)
endif()
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	set_property(SOURCE src/allocator.cpp src/buffer.cpp APPEND PROPERTY COMPILE_OPTIONS
		-Wno-unreachable-code # Memory allocation functions don't return nullptr in ASAN-less Clang builds.
		)
endif()
source_group("include" FILES ${HEADERS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_library(seir_base STATIC ${HEADERS} ${SOURCES})
add_library(Seir::base ALIAS seir_base)
target_include_directories(seir_base PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
seir_target(seir_base FOLDER libs/base STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
seir_install(seir_base EXPORT SeirTargets)
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/seir_base)
