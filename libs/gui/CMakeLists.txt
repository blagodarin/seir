# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(HEADERS
	include/seir_gui/context.hpp
	include/seir_gui/font.hpp
	include/seir_gui/frame.hpp
	)
set(SOURCES
	src/context.cpp
	src/context_impl.cpp
	src/context_impl.hpp
	src/font.cpp
	src/frame.cpp
	)
source_group("include" FILES ${HEADERS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_library(seir_gui STATIC ${HEADERS} ${SOURCES})
add_library(Seir::gui ALIAS seir_gui)
target_include_directories(seir_gui PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(seir_gui PUBLIC Seir::app PRIVATE Seir::data Seir::graphics Seir::image Seir::renderer Freetype::Freetype)
seir_target(seir_gui FOLDER libs/gui STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
seir_install(seir_gui EXPORT SeirTargets)
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/seir_gui)
