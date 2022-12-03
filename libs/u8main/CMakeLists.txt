# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(HEADERS
	include/seir_u8main/u8main.hpp
	)
if(WIN32)
	set(SOURCES
		src/u8main_windows.cpp
		)
	if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
		set_property(SOURCE src/u8main_windows.cpp APPEND PROPERTY COMPILE_OPTIONS
			/wd4668 # '___' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
			)
	endif()
else()
	set(SOURCES
		src/u8main_posix.cpp
		)
endif()
source_group("include" FILES ${HEADERS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_library(seir_u8main STATIC ${HEADERS} ${SOURCES})
add_library(Seir::u8main ALIAS seir_u8main)
target_include_directories(seir_u8main PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_link_libraries(seir_u8main PRIVATE Seir::base)
seir_target(seir_u8main FOLDER libs/u8main STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
seir_install(seir_u8main EXPORT SeirTargets)
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/seir_u8main)
