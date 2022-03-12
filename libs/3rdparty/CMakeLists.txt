# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(HEADERS
	include/seir_3rdparty/fmt.hpp
	)
source_group("include" FILES ${HEADERS})
add_library(seir_3rdparty INTERFACE ${HEADERS})
add_library(Seir::3rdparty ALIAS seir_3rdparty)
target_include_directories(seir_3rdparty INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
seir_target(seir_3rdparty FOLDER libs/3rdparty STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
seir_install(seir_3rdparty EXPORT SeirTargets)
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/seir_3rdparty)
