# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(HEADERS
	include/seir_model/mesh_data.hpp
	include/seir_model/mesh_format.hpp
	)
set(SOURCES
	src/mesh_data.cpp
	src/obj.cpp
	)
source_group("include" FILES ${HEADERS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_library(seir_model STATIC ${HEADERS} ${SOURCES})
add_library(Seir::model ALIAS seir_model)
target_include_directories(seir_model PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(seir_model PUBLIC Seir::base PRIVATE Seir::io Seir::math)
seir_target(seir_model FOLDER libs/model STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
seir_install(seir_model EXPORT SeirTargets)
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/seir_model)
