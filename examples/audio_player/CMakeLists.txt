# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(SOURCES
	src/main.cpp
	)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_executable(audio_player ${SOURCES})
target_link_libraries(audio_player PRIVATE Seir::audio)
seir_target(audio_player FOLDER examples STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
