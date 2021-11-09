# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

seir_folder(audio)
source_group("src" REGULAR_EXPRESSION ".*\\.(hpp|cpp)$")
source_group("include" REGULAR_EXPRESSION "/include/")
add_library(audio STATIC
	include/seir_audio/processing.hpp
	src/processing.cpp
	)
add_library(Seir::audio ALIAS audio)
target_include_directories(audio PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(audio PUBLIC Seir::base)
set_target_properties(audio PROPERTIES
	MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<NOT:$<BOOL:${SEIR_STATIC_RUNTIME}>>:DLL>"
	OUTPUT_NAME seir_audio)
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_compile_options(audio PRIVATE -msse4.1) # TODO: Use target platform detection.
endif()
seir_install(audio EXPORT SeirTargets)
install(DIRECTORY include/seir_audio DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
