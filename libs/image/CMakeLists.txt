# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(HEADERS
	include/seir_image/image.hpp
	include/seir_image/utils.hpp
	)
set(SOURCES
	src/bmp.hpp
	src/format.hpp
	src/image.cpp
	src/utils.cpp
	)
if(SEIR_IMAGE_BMP)
	list(APPEND SOURCES src/format_bmp.cpp)
endif()
if(SEIR_IMAGE_DDS)
	list(APPEND SOURCES src/format_dds.cpp)
endif()
if(SEIR_IMAGE_ICO)
	list(APPEND SOURCES src/format_ico.cpp)
endif()
if(SEIR_IMAGE_JPEG)
	list(APPEND SOURCES src/format_jpeg.cpp)
	if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		set_property(SOURCE src/format_jpeg.cpp APPEND PROPERTY COMPILE_OPTIONS
			-Wno-disabled-macro-expansion # Clang issues a warning for libc++'s setjmp.
			)
	elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
		set_property(SOURCE src/format_jpeg.cpp APPEND PROPERTY COMPILE_OPTIONS
			/wd4324 # '`anonymous-namespace'::JpegErrorHandler': structure was padded due to alignment specifier
			/wd4611 # interaction between '_setjmp' and C++ object destruction is non-portable
			)
	endif()
endif()
if(SEIR_IMAGE_PNG)
	list(APPEND SOURCES src/format_png.cpp)
endif()
if(SEIR_IMAGE_TGA)
	list(APPEND SOURCES src/format_tga.cpp)
endif()
if(SEIR_IMAGE_WEBP)
	list(APPEND SOURCES src/format_webp.cpp)
endif()
source_group("include" FILES ${HEADERS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_library(seir_image STATIC ${HEADERS} ${SOURCES})
add_library(Seir::image ALIAS seir_image)
target_compile_definitions(seir_image PUBLIC
	SEIR_IMAGE_BMP=$<BOOL:${SEIR_IMAGE_BMP}>
	SEIR_IMAGE_DDS=$<BOOL:${SEIR_IMAGE_DDS}>
	SEIR_IMAGE_ICO=$<BOOL:${SEIR_IMAGE_ICO}>
	SEIR_IMAGE_JPEG=$<BOOL:${SEIR_IMAGE_JPEG}>
	SEIR_IMAGE_PNG=$<BOOL:${SEIR_IMAGE_PNG}>
	SEIR_IMAGE_TGA=$<BOOL:${SEIR_IMAGE_TGA}>
	SEIR_IMAGE_WEBP=$<BOOL:${SEIR_IMAGE_WEBP}>
	)
target_include_directories(seir_image PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(seir_image PUBLIC Seir::base PRIVATE Seir::data
	$<$<BOOL:${SEIR_IMAGE_JPEG}>:JPEG::JPEG>
	$<$<BOOL:${SEIR_IMAGE_WEBP}>:WebP::webpdecoder>
	)
seir_target(seir_image FOLDER libs/image STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
seir_install(seir_image EXPORT SeirTargets)
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/seir_image)
