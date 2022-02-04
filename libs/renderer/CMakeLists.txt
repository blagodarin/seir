# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(HEADERS
	include/seir_renderer/renderer.hpp
	)
set(SOURCES
	src/dummy/renderer.cpp
	)
source_group("include" FILES ${HEADERS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_library(seir_renderer STATIC ${HEADERS} ${SOURCES})
add_library(Seir::renderer ALIAS seir_renderer)
target_include_directories(seir_renderer PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(seir_renderer PRIVATE Seir::app)
seir_target(seir_renderer FOLDER libs/renderer STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
seir_install(seir_renderer EXPORT SeirTargets)
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/seir_renderer)
