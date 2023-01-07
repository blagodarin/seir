# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(SOURCES
	src/main.cpp
	)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_executable(window ${SOURCES})
target_link_libraries(window PRIVATE Seir::app Seir::image Seir::renderer Seir::u8main fmt::fmt)
set_target_properties(window PROPERTIES WIN32_EXECUTABLE $<NOT:$<CONFIG:Debug>>)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_compile_options(window PRIVATE -Wno-unused-macros) # macro "__OPTIMIZE__" is not used
endif()
seir_target(window FOLDER examples STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
if(WIN32)
	target_sources(window PRIVATE window.rc)
	source_group("res" FILES window.rc)
endif()
