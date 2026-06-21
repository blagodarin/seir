# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(SOURCES
	src/main.cpp
	)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_executable(minimal_ui ${SOURCES})
target_compile_definitions(minimal_ui PRIVATE SEIR_DATA_DIR="${SEIR_DATA_DIR}/")
target_link_libraries(minimal_ui PRIVATE Seir::io Seir::u8main Seir::ui)
set_target_properties(minimal_ui PROPERTIES WIN32_EXECUTABLE $<NOT:$<CONFIG:Debug>>)
seir_target(minimal_ui FOLDER examples STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
if(WIN32)
	target_sources(minimal_ui PRIVATE ${SEIR_DATA_DIR}/icon.rc)
	source_group("" FILES ${SEIR_DATA_DIR}/icon.rc)
endif()
