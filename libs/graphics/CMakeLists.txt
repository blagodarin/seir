# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(HEADERS
	include/seir_graphics/color.hpp
	include/seir_graphics/margins.hpp
	include/seir_graphics/marginsf.hpp
	include/seir_graphics/point.hpp
	include/seir_graphics/quadf.hpp
	include/seir_graphics/rect.hpp
	include/seir_graphics/rectf.hpp
	include/seir_graphics/size.hpp
	include/seir_graphics/sizef.hpp
	)
source_group("include" FILES ${HEADERS})
add_library(seir_graphics INTERFACE ${HEADERS})
add_library(Seir::graphics ALIAS seir_graphics)
target_include_directories(seir_graphics INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(seir_graphics INTERFACE Seir::math)
seir_target(seir_graphics FOLDER libs/graphics STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
seir_install(seir_graphics EXPORT SeirTargets)
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/seir_graphics)
