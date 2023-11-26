# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(HEADERS
	include/seir_app/app.hpp
	include/seir_app/events.hpp
	include/seir_app/key.hpp
	include/seir_app/window.hpp
	)
set(SOURCES
	)
if(WIN32)
	list(APPEND SOURCES
		src/windows/app.cpp
		src/windows/app.hpp
		src/windows/helpers.cpp
		src/windows/helpers.hpp
		src/windows/window.cpp
		src/windows/window.hpp
		)
else()
	# TODO: Add XCB implementation.
	list(APPEND SOURCES
		src/stub/app.cpp
		src/stub/window.cpp
		)
endif()
source_group("include" FILES ${HEADERS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_library(seir_app STATIC ${HEADERS} ${SOURCES})
add_library(Seir::app ALIAS seir_app)
target_include_directories(seir_app PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(seir_app PUBLIC Seir::base PRIVATE Seir::image)
seir_target(seir_app FOLDER libs/app STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
seir_install(seir_app EXPORT SeirTargets)
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/seir_app)
