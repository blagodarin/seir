# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(HEADERS
	include/seir_image/image.hpp
	)
set(SOURCES
	src/image.cpp
	)
source_group("include" FILES ${HEADERS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_library(seir_image STATIC ${HEADERS} ${SOURCES})
add_library(Seir::image ALIAS seir_image)
target_include_directories(seir_image PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
seir_target(seir_image FOLDER libs/image STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
seir_install(seir_image EXPORT SeirTargets)
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/seir_image)
