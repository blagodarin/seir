# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(SOURCES
	src/main.cpp
	)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_executable(flatfield ${SOURCES})
target_compile_definitions(flatfield PRIVATE SEIR_DATA_DIR="${SEIR_DATA_DIR}/")
target_link_libraries(flatfield PRIVATE Seir::gui Seir::io Seir::renderer Seir::u8main)
set_target_properties(flatfield PROPERTIES WIN32_EXECUTABLE $<NOT:$<CONFIG:Debug>>)
seir_target(flatfield FOLDER examples STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
if(WIN32)
	target_sources(flatfield PRIVATE ${SEIR_DATA_DIR}/icon.rc)
	source_group("" FILES ${SEIR_DATA_DIR}/icon.rc)
endif()
