# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(HEADERS
	include/seir_main/u8main.hpp
	)
if(WIN32)
	set(SOURCES
		src/u8main_windows.cpp
		)
else()
	set(SOURCES
		src/u8main_posix.cpp
		)
endif()
source_group("include" FILES ${HEADERS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_library(seir_main STATIC ${HEADERS} ${SOURCES})
add_library(Seir::main ALIAS seir_main)
target_include_directories(seir_main PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_link_libraries(seir_main PRIVATE Seir::base)
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_options(seir_main PRIVATE
		/wd4668 # '___' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
		/wd5039 # pointer or reference to potentially throwing function passed to 'extern "C"' function under -EHc. Undefined behavior may occur if this function throws an exception.
		)
endif()
seir_target(seir_main FOLDER main STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
seir_install(seir_main EXPORT SeirTargets INCLUDE_DIRECTORY include/seir_main)
