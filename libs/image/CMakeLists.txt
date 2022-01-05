# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(HEADERS
	include/seir_image/image.hpp
	)
set(SOURCES
	src/format.hpp
	src/image.cpp
	)
if(SEIR_IMAGE_BMP OR SEIR_IMAGE_ICO)
	list(APPEND SOURCES src/format_bmp.cpp)
endif()
if(SEIR_IMAGE_TGA)
	list(APPEND SOURCES src/format_tga.cpp)
endif()
source_group("include" FILES ${HEADERS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_library(seir_image STATIC ${HEADERS} ${SOURCES})
add_library(Seir::image ALIAS seir_image)
target_compile_definitions(seir_image PUBLIC
	SEIR_IMAGE_BMP=$<BOOL:${SEIR_IMAGE_BMP}>
	SEIR_IMAGE_ICO=$<BOOL:${SEIR_IMAGE_ICO}>
	SEIR_IMAGE_TGA=$<BOOL:${SEIR_IMAGE_TGA}>
	)
target_include_directories(seir_image PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(seir_image PUBLIC Seir::base PRIVATE Seir::data)
seir_target(seir_image FOLDER libs/image STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
seir_install(seir_image EXPORT SeirTargets)
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/seir_image)
