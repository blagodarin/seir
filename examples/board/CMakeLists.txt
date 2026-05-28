# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(SOURCES
	src/main.cpp
	)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_executable(board ${SOURCES})
target_compile_definitions(board PRIVATE
	SEIR_DATA_DIR="${SEIR_DATA_DIR}/"
	LOCAL_DATA_DIR="${CMAKE_CURRENT_LIST_DIR}/data/"
	)
target_link_libraries(board PRIVATE Seir::gui Seir::image Seir::io Seir::model Seir::renderer Seir::u8main)
set_target_properties(board PROPERTIES WIN32_EXECUTABLE $<NOT:$<CONFIG:Debug>>)
seir_target(board FOLDER examples STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
if(WIN32)
	target_sources(board PRIVATE ${SEIR_DATA_DIR}/icon.rc)
	source_group("" FILES ${SEIR_DATA_DIR}/icon.rc)
endif()
if(SEIR_RENDERER_BACKEND STREQUAL "Vulkan")
	seir_embed_spirv(board
		VERTEX src/board_vs.glsl
		FRAGMENT src/board_fs.glsl
		)
endif()
