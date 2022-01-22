# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(HEADERS
	include/seir_app/app.hpp
	)
set(SOURCES
	src/app.cpp
	)
source_group("include" FILES ${HEADERS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_library(seir_app STATIC ${HEADERS} ${SOURCES})
add_library(Seir::app ALIAS seir_app)
target_include_directories(seir_app PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
seir_target(seir_app FOLDER libs/app STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
seir_install(seir_app EXPORT SeirTargets)
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/seir_app)
