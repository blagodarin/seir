# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(HEADERS
	include/seir_data/blob.hpp
	include/seir_data/reader.hpp
	)
if(WIN32)
	list(APPEND SOURCES src/blob_windows.cpp)
else()
	list(APPEND SOURCES src/blob_posix.cpp)
	set_property(SOURCE src/blob_posix.cpp APPEND PROPERTY COMPILE_DEFINITIONS _FILE_OFFSET_BITS=64)
endif()
source_group("include" FILES ${HEADERS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_library(seir_data STATIC ${HEADERS} ${SOURCES})
add_library(Seir::data ALIAS seir_data)
target_include_directories(seir_data PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(seir_data PUBLIC Seir::base)
seir_target(seir_data FOLDER data STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
seir_install(seir_data EXPORT SeirTargets)
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/seir_data)
