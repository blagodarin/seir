# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(HEADERS
	include/seir_math/euler.hpp
	include/seir_math/mat.hpp
	include/seir_math/vec.hpp
	)
set(SOURCES
	src/mat.cpp
	src/vec.cpp
	)
source_group("include" FILES ${HEADERS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_library(seir_math STATIC ${HEADERS} ${SOURCES})
add_library(Seir::math ALIAS seir_math)
target_include_directories(seir_math PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	target_compile_options(seir_math PUBLIC -Wno-float-equal)
endif()
seir_target(seir_math FOLDER libs/math STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
seir_install(seir_math EXPORT SeirTargets)
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/seir_math)
