# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(HEADERS
	include/seir_data/archive.hpp
	include/seir_data/compression.hpp
	include/seir_data/storage.hpp
	)
set(SOURCES
	src/archive.cpp
	src/archive.hpp
	src/archive_seir.cpp
	src/compression.cpp
	src/compression.hpp
	src/storage.cpp
	)
if(SEIR_COMPRESSION_ZLIB)
	list(APPEND SOURCES src/compression_zlib.cpp)
	set_property(SOURCE src/compression_zlib.cpp APPEND PROPERTY COMPILE_DEFINITIONS ZLIB_CONST)
endif()
if(SEIR_COMPRESSION_ZSTD)
	list(APPEND SOURCES src/compression_zstd.cpp)
endif()
source_group("include" FILES ${HEADERS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_library(seir_data STATIC ${HEADERS} ${SOURCES})
add_library(Seir::data ALIAS seir_data)
target_compile_definitions(seir_data PUBLIC
	SEIR_COMPRESSION_ZLIB=$<BOOL:${SEIR_COMPRESSION_ZLIB}>
	SEIR_COMPRESSION_ZSTD=$<BOOL:${SEIR_COMPRESSION_ZSTD}>
	)
target_include_directories(seir_data PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(seir_data PUBLIC Seir::base PRIVATE Seir::io
	$<$<BOOL:${SEIR_COMPRESSION_ZLIB}>:ZLIB::ZLIB>
	$<$<BOOL:${SEIR_COMPRESSION_ZSTD}>:zstd::libzstd_static>
	)
seir_target(seir_data FOLDER libs/data STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
seir_install(seir_data EXPORT SeirTargets)
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/seir_data)
