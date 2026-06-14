# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(HEADERS
	include/seir_ui/context.hpp
	include/seir_ui/font.hpp
	include/seir_ui/frame.hpp
	include/seir_ui/layout.hpp
	include/seir_ui/style.hpp
	)
set(SOURCES
	src/context.cpp
	src/context_impl.cpp
	src/context_impl.hpp
	src/font.cpp
	src/frame.cpp
	src/keyboard_item.cpp
	src/keyboard_item.hpp
	src/layout.cpp
	src/style.cpp
	)
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	set_property(SOURCE src/frame.cpp APPEND PROPERTY COMPILE_OPTIONS
		-Wno-switch-enum
		)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	set_property(SOURCE src/frame.cpp APPEND PROPERTY COMPILE_OPTIONS
		/wd4061 # enumerator '___' in switch of enum 'seir::Key' is not explicitly handled by a case label
		)
	set_property(SOURCE src/layout.cpp APPEND PROPERTY COMPILE_OPTIONS
		/wd4355 # 'this': used in base member initializer list
		)
endif()
source_group("include" FILES ${HEADERS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_library(seir_ui STATIC ${HEADERS} ${SOURCES})
add_library(Seir::ui ALIAS seir_ui)
target_include_directories(seir_ui PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(seir_ui PUBLIC Seir::app Seir::graphics PRIVATE Seir::image Seir::io Seir::renderer Freetype::Freetype)
seir_target(seir_ui FOLDER libs/ui STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
seir_install(seir_ui EXPORT SeirTargets)
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/seir_ui)
