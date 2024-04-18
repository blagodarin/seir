# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(HEADERS
	include/seir_serialization/st_reader.hpp
	include/seir_serialization/st_stream.hpp
	include/seir_serialization/st_writer.hpp
	)
set(SOURCES
	src/st_reader.cpp
	src/st_stream.cpp
	src/st_writer.cpp
	)
source_group("include" FILES ${HEADERS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_library(seir_serialization STATIC ${HEADERS} ${SOURCES})
add_library(Seir::serialization ALIAS seir_serialization)
target_include_directories(seir_serialization PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(seir_serialization PRIVATE Seir::data)
seir_target(seir_serialization FOLDER libs/serialization STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
seir_install(seir_serialization EXPORT SeirTargets)
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/seir_serialization)
