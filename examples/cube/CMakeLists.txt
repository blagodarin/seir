# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(SOURCES
	src/main.cpp
	)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_executable(cube ${SOURCES})
target_compile_definitions(cube PRIVATE SEIR_DATA_DIR="${PROJECT_SOURCE_DIR}/data/")
target_link_libraries(cube PRIVATE Seir::app Seir::data Seir::graphics Seir::gui Seir::image Seir::renderer Seir::u8main fmt::fmt)
set_target_properties(cube PROPERTIES WIN32_EXECUTABLE $<NOT:$<CONFIG:Debug>>)
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	target_compile_options(cube PRIVATE
		-Wno-exit-time-destructors
		-Wno-global-constructors
		)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_compile_options(cube PRIVATE
		-Wno-unused-macros # macro "__OPTIMIZE__" is not used
		)
endif()
seir_target(cube FOLDER examples STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
if(WIN32)
	target_sources(cube PRIVATE cube.rc)
	source_group("res" FILES cube.rc)
endif()
if(SEIR_RENDERER STREQUAL "Vulkan")
	seir_embed_spirv(cube
		VERTEX src/vertex_shader.glsl
		FRAGMENT src/fragment_shader.glsl
		)
endif()
