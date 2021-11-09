# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

seir_folder(audio)
set(HEADERS
	include/seir_audio/player.hpp
	include/seir_audio/processing.hpp
	)
set(SOURCES
	src/backend.hpp
	src/player.cpp
	src/processing.cpp
	)
if(WIN32)
	list(APPEND SOURCES src/backend_wasapi.cpp)
	set_property(SOURCE src/player.cpp APPEND PROPERTY COMPILE_OPTIONS
		/wd4355 # 'this': used in base member initializer list
		)
else()
	list(APPEND SOURCES src/backend_alsa.cpp)
endif()
source_group("include" FILES ${HEADERS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_library(audio STATIC ${HEADERS} ${SOURCES})
add_library(Seir::audio ALIAS audio)
target_include_directories(audio PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(audio PUBLIC Seir::base Threads::Threads $<$<NOT:$<BOOL:${WIN32}>>:ALSA::ALSA>)
set_target_properties(audio PROPERTIES
	MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<NOT:$<BOOL:${SEIR_STATIC_RUNTIME}>>:DLL>"
	OUTPUT_NAME seir_audio)
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_compile_options(audio PRIVATE -msse4.1) # TODO: Use target platform detection.
endif()
seir_install(audio EXPORT SeirTargets)
install(DIRECTORY include/seir_audio DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
