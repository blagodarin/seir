# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(HEADERS
	include/seir_package/archive.hpp
	include/seir_package/storage.hpp
	)
set(SOURCES
	src/archive.cpp
	src/archive.hpp
	src/archive_seir.cpp
	src/storage.cpp
	)
source_group("include" FILES ${HEADERS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_library(seir_package STATIC ${HEADERS} ${SOURCES})
add_library(Seir::package ALIAS seir_package)
target_include_directories(seir_package PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(seir_package PUBLIC Seir::base PRIVATE Seir::compression Seir::io)
seir_target(seir_package FOLDER libs/package STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
seir_install(seir_package EXPORT SeirTargets)
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/seir_package)
