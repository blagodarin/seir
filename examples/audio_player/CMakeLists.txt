# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(SOURCES
	src/main.cpp
	)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_executable(audio_player ${SOURCES})
target_compile_definitions(audio_player PRIVATE SEIR_DATA_DIR="${SEIR_DATA_DIR}/")
target_link_libraries(audio_player PRIVATE Seir::audio Seir::io Seir::u8main Seir::ui)
set_target_properties(audio_player PROPERTIES WIN32_EXECUTABLE $<NOT:$<CONFIG:Debug>>)
seir_target(audio_player FOLDER examples STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
if(WIN32)
	target_sources(audio_player PRIVATE ${SEIR_DATA_DIR}/icon.rc)
	source_group("" FILES ${SEIR_DATA_DIR}/icon.rc)
endif()
