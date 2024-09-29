# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(HEADERS
	include/seir_io/blob.hpp
	include/seir_io/buffer_blob.hpp
	include/seir_io/buffer_writer.hpp
	include/seir_io/paths.hpp
	include/seir_io/reader.hpp
	include/seir_io/save_file.hpp
	include/seir_io/temporary.hpp
	include/seir_io/writer.hpp
	)
set(SOURCES
	src/buffer_writer.cpp
	src/writer.cpp
	)
if(WIN32)
	list(APPEND SOURCES
		src/windows/file.cpp
		src/windows/paths.cpp
		src/windows/utils.hpp
		)
	if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
		set_property(SOURCE src/windows/paths.cpp APPEND PROPERTY COMPILE_OPTIONS
			/wd5039 # pointer or reference to potentially throwing function passed to 'extern "C"' function under -EHc
			)
	endif()
else()
	list(APPEND SOURCES
		src/posix/file.cpp
		src/posix/paths.cpp
		)
	set_property(SOURCE src/posix/file.cpp APPEND PROPERTY COMPILE_DEFINITIONS _FILE_OFFSET_BITS=64)
endif()
source_group("include" FILES ${HEADERS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_library(seir_io STATIC ${HEADERS} ${SOURCES})
add_library(Seir::io ALIAS seir_io)
target_include_directories(seir_io PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(seir_io PUBLIC Seir::base)
seir_target(seir_io FOLDER libs/io STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
seir_install(seir_io EXPORT SeirTargets)
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/seir_io)
